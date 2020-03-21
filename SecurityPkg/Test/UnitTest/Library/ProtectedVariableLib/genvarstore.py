## @file
# Generate variable storage with variable encrypted.
#
# Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
import json
import struct

from collections import OrderedDict

#from Crypto.Util.Padding import pad, unpad
from Crypto.Cipher import AES
from Crypto.Protocol.KDF import HKDF
from Crypto.Hash import SHA256, HMAC

from dict_struct import DictStruct

args = None
tohex = lambda b: "0x%02X" % b
hmac_var_name = bytearray("MetaDataHmacVar\x00", "utf-16-le")
unenc_var_names = [
    hmac_var_name,
    bytearray("VarErrorFlag\x00", "utf-16-le")
    ]
enc_header = "IIII16B"

def checksum(data):
    sum = 0
    i = 0
    while (i < len(data)):
        sum += int.from_bytes(data[i:i+2], "little")
        i += 2
    return sum & 0xffff

def get_var_store_info():
    if args:
        with open(args.desc_file) as fd:
            var_store_info = json.load(fd, object_pairs_hook=OrderedDict)
    else:
        import win32com.client
        autoit = win32com.client.Dispatch("AutoItX3.Control")
        info_in_json = autoit.ClipGet().replace('\r', '').replace('\n', '')
        var_store_info = json.loads(info_in_json, object_pairs_hook=OrderedDict)

    return var_store_info

def gen_enc_key(rootkey, var):
    info = bytearray()

    # Name || ":" || Guid || ":" || Attr || "VAR_ENC_KEY"
    info.extend(var["Name"])
    info.extend(bytearray(":", "utf-16-le"))
    info.extend(var["VendorGuid"])
    info.extend(bytearray(":", "utf-16-le"))
    info.extend(var['Attributes'])
    info.extend(bytearray("VAR_ENC_KEY", "utf-16-le"))

    # HKDF256
    key = HKDF(rootkey, len(rootkey), b"", SHA256, 1, info)

    return key

def gen_hmac_key(rootkey):
    # "HMAC_KEY"
    info = bytearray("HMAC_KEY", "utf-16-le")

    # HKDF256
    print("Info(%d):" % len(info), info.hex())
    key = HKDF(rootkey, len(rootkey), b"", SHA256, 1, info)

    print("RootKey:", rootkey.hex())
    print("HmacKey:", key.hex())
    return key

def enc_var(rootkey, iv, var):
    plain_data_size = len(var["Data"])
    key = gen_enc_key(rootkey, var)
    cipher = AES.new(key, AES.MODE_CBC, iv)
    data = var["Data"]
    if (len(data) % AES.block_size) != 0:
        padding = AES.block_size - (len(data) % AES.block_size)
        data[len(data) :] = b'\x0f' * padding

    data = cipher.encrypt(data)
    cipher_data_size = len(data)

    header = struct.pack(enc_header, 6, 32, plain_data_size, cipher_data_size, *iv)
    print ("    EncHeader:  ", header.hex())

    return header + data

def get_hmac_data(var):
    # || ":" || Name
    # || ":" || VendorGuid || Attributes || DataSize
    # || ":" || CipherData
    # || ":" || PubKeyIndex || MonotonicCount || TimeStamp
    sep = bytearray(":", "utf-16-le")

    hmac_data = bytearray()
    hmac_data.extend(sep)
    hmac_data.extend(var["Name"])
    hmac_data.extend(sep)
    hmac_data.extend(var["VendorGuid"])
    hmac_data.extend(var["Attributes"])
    hmac_data.extend(var["CipherDataSize"])
    hmac_data.extend(sep)
    hmac_data.extend(var["CipherData"])
    hmac_data.extend(sep)
    hmac_data.extend(var["PubKeyIndex"])
    hmac_data.extend(var["MonotonicCount"])
    hmac_data.extend(var["TimeStamp"])

    print("    Variable Data for Hmac:", hmac_data.hex())
    return hmac_data

def to_c_array(data, c_variable):
    c_array_s = 'unsigned char %s[%d] = {\n' % (c_variable, len(data))
    data_hex = list(map(tohex, data))
    i = 0
    while (i < len(data)):
        if i != 0:
            c_array_s += ',\n'
        c_array_s += ', '.join(data_hex[i:i+16])
        i+=16
    c_array_s += '\n};\n\n'

    return c_array_s;

def gen_var_store():
    var_store_info = get_var_store_info()
    var_store = DictStruct(var_store_info, 0xff)

    rootkey = var_store["RootKey"]
    iv = var_store["InitVec"]

    # HMAC: Count || Var1 || Var2 || ... || VarN
    # Var1 = || ":" || Name || ":" || Guid || Attr || DataSize || ":" || CipherData
    #        || ":" || PubKeyIndex || MonotonicCount || TimeStamp
    hmac_var_data = bytearray()
    hmac_var_key = gen_hmac_key(rootkey)
    hmac_var_index = None
    var_rpmc = var_store["RpmcCounter"]
    print("RPMC =", var_rpmc.hex())

    #
    # Encrypt variable data
    #
    i = 0
    while True:
        var_index = "Variables.%d" % i
        var_name = var_store[var_index + ".Name"]
        if var_name == None:
            break

        print("[%d] %s" % (i, var_name.decode("utf-16-le")))

        encrypt_flag = int.from_bytes(var_store[var_index + ".Reserved"], 'little')
        if var_name == hmac_var_name and encrypt_flag:
            print("    @%d" % i)
            hmac_var_index = i

        if (not encrypt_flag) or (var_name in unenc_var_names):
            print("    Offset:", hex(var_store.offset(var_index + ".StartId")), var_store[var_index + ".StartId"].hex())
            i += 1
            continue

        var = {}
        var['Name'] = var_name
        var['Data'] = var_store[var_index + ".Data"]
        var['DataSize'] = var_store[var_index + ".DataSize"]
        var['VendorGuid'] = var_store[var_index + ".VendorGuid"]
        var['Attributes'] = var_store[var_index + ".Attributes"]
        var['MonotonicCount'] = var_store[var_index + ".MonotonicCount"]
        var['TimeStamp'] = var_store[var_index + ".TimeStamp"]
        var['PubKeyIndex'] = var_store[var_index + ".PubKeyIndex"]
        var['MonotonicCount'] = var_store[var_index + ".MonotonicCount"]

        var["CipherData"] = enc_var(rootkey, iv, var)
        var["CipherDataSize"] = bytes([len(var["CipherData"]), 0, 0, 0])
        print("    Cipher data:", var["CipherData"].hex())
        var_store[var_index + ".DataSize"] = len(var["CipherData"])
        var_store[var_index + ".Reserved"] = 0
        var_store[var_index + ".Data"]     = var["CipherData"]

        print("    Offset:", hex(var_store.offset(var_index + ".StartId")), var_store[var_index + ".StartId"].hex())

        hmac_var_data.extend(get_hmac_data(var))

        i += 1
    #
    # Put RPMC at the end
    #
    hmac_var_data.extend(var_rpmc)

    #
    # Fill-up the checksum field in FV header
    #
    var_store["FvHeader.Checksum"] = 0
    fv_header = var_store["FvHeader"]
    var_store["FvHeader.Checksum"] = 0x10000 - checksum(fv_header)

    #
    # Fill-up MetaDataHmacVariable
    #
    if hmac_var_index != None:
        hmac = HMAC.new(hmac_var_key, digestmod=SHA256)
        hmac.update(hmac_var_data)
        var_store["Variables.%d.Data" % hmac_var_index] = hmac.digest()
        print("HMAC: ", hmac.hexdigest())

    if args.fv:
        var_store.to_file(args.fv)

    if args.c_file:
        cs = "UINT32 %s_rpmc = 0x%x;\n\n" % (args.tc, int.from_bytes(var_rpmc, 'little'))
        cs += to_c_array(iv, "%s_ivec" % args.tc)
        cs += to_c_array(var_store.data, "%s_var_fv" % args.tc)
        with open(args.c_file, 'wt') as fd:
            fd.write(cs)

if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser(description='Generate variable store FV.')
    parser.add_argument('-d', '--desc-file',
                        default=r'C:\src\combo\edk2\github-jw36\SecurityPkg\Test\UnitTest\Library\ProtectedVariableLib\var_store.json',
                        help='Variable store description file in jason format')
    parser.add_argument('-o', '--fv', default="var_store_enc.fv", help='Firmware volume file to outoupt')
    parser.add_argument('-c', '--c-file', default="", help='C array format to outoupt')
    parser.add_argument('-n', '--tc', default="tc", help='Test case number')

    args = parser.parse_args()

    gen_var_store()
