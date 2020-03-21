## @file
# Json representation of data structure
#
# Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
import re
import sys
import copy
import json
import uuid
import struct

from collections import OrderedDict

DATA_SIZE = {
    1   :   'B',
    2   :   'H',
    3   :   'I',
    4   :   'I',
    5   :   'Q',
    6   :   'Q',
    7   :   'Q',
    8   :   'Q'
}

class DictStruct(object):
    def __init__(self, desc_dict, padding=0):
        self._desc = desc_dict
        self._struct = desc_dict["__struct__"]
        self._padding = bytearray([padding])
        self._data = self._parse_data(self._struct, 0)
        self._meta_info = self._desc["__metainfo__"]

    def __len__(self):
        return len(self._data)

    def __getitem__(self, key):
        data = None
        if key in self._meta_info:
            data = self._meta_info[key]
            if ':' in data:
                _,data = self._get_desc_data(data, 0)
        else:
            data = self._get_data(key)
        return data

    def __setitem__(self, key, value):
        if key not in self._meta_info:
            self._set_data(key, value)

    def _offsetof_(self, field):
        offset = 0
        fields = field.split('.')
        data = self._data
        for f in fields:
            if "@fields@" not in data or f not in data["@fields@"]:
                return None

            for subf in data["@fields@"]:
                if subf == f:
                    break
                offset += self._sizeof_(data["@fields@"][subf])
            data = data["@fields@"][f]
        return offset


    def _sizeof_(self, data, offset=0):
        size = 0
        if "@buf@" in data:
            size += len(data["@buf@"])
        elif "@fields@" in data:
            for f in data["@fields@"]:
                size += self._sizeof_(data["@fields@"][f])
        elif "@desc@" in data and data["@desc@"]:
            size += len(self._get_desc_data(data["@desc@"], offset)[1])
        return size

    def _fixup_(self, data=None, offset=0):
        if data == None:
            data = self._data

        if "@desc@" in data and data["@desc@"].startswith("@:"):
            _,data["@buf@"] = self._get_desc_data(data["@desc@"], offset)

        if "@buf@" in data:
            return len(data["@buf@"])

        size = 0
        for f in data["@fields@"]:
            size += self._fixup_(data["@fields@"][f], offset + size)

        return size

    def _parse_data(self, desc, offset=0):
        data = OrderedDict()
        if not desc:
            return data

        if isinstance(desc, dict):
            for name in desc:
                desc[name] = self._parse_data(desc[name], offset)
                offset += self._sizeof_(desc[name])
            data["@fields@"] = desc
        elif type(desc) in [list, tuple]:
            fields = OrderedDict()
            index = 0
            for d in desc:
                fields[str(index)] = self._parse_data(d, offset)
                offset += self._sizeof_(fields[str(index)])
                index += 1
            data["@fields@"] = fields
        else:
            assert (type(desc) in [str, bytes, bytearray])
            data["@desc@"], data["@buf@"] = self._get_desc_data(desc, offset)
            #print("%x: %s -> %s" % (offset, data["@desc@"], data["@buf@"]))
        return data

    def checksum(self, data):
        sum = 0
        i = 0
        while (i < len(data)):
            sum += int.from_bytes(data[i:i+2], "little")
            i += 2
        return sum & 0xffff

    def _align_(self, alignment, offset=0):
        alignment = int(alignment, 0)
        if not offset:
            offset = len(self._data)
        if alignment == 1:
            return bytearray()
        else:
            return self._padding * (((~offset) + 1) & (alignment - 1))

    def _to_bytes(self, s):
        s = s.replace("0x", "").replace(",", "")
        s = s.replace("{", "").replace("}", "")
        s = s.replace("/", "").replace("\\", "")
        return bytearray.fromhex(s)

    def _get_desc_data(self, data_desc, offset):
        if ':' not in data_desc:
            fmt = data_desc.strip()
            fdt = ''
        else:
            fmt, fdt = data_desc.split(':', 1)
            fmt = fmt.strip()
            fdt = fdt.strip()

        # Special formats other than struct.pack supported
        if fmt == 'G':      # GUID
            if fdt:
                buf= uuid.UUID("{%s}" % fdt).bytes_le
            else:
                buf = self._padding * 16
        elif fmt == 'U':    # Unicode string
            buf = bytearray(fdt, "utf-16-le")
            buf.extend(b"\x00\x00")
        elif fmt == 'S':    # Ascii string
            buf = bytearray(fdt, "ascii")
            buf.extend(b"\x00")
        elif fmt == '@':    # Function calling
            func, args = fdt.split('(', 1)
            func = func.strip()
            args = args.split(')')[0].strip()
            args = list(map(str.strip, args.split(',')))
            buf = getattr(self, func)(*args, offset)
            fmt = data_desc
        elif fmt:
            size = struct.calcsize(fmt.strip())
            if not fdt:
                fdt = ' '.join(['00'] * size)
                new_fmt = '%dB' % size
            else:
                fdt = fdt.replace('{', '').replace('}', '')
                new_fmt = fmt

            to_int = lambda x: int(x, 16)
            fdt_list = re.split(r"[, \r\n\t]+", fdt)
            fdt = list(map(to_int, fdt_list))
            buf = bytearray(struct.pack(new_fmt, *fdt))
        else:
            # If no type given, suppose byte array.
            buf = self._to_bytes(fdt)

        return fmt, buf

    # Support updating fields with different size
    def _get_bytes(self, data, until=''):
        buf = bytearray()

        if "@buf@" in data:
            #print(data["@buf@"])
            buf[len(buf):] = data["@buf@"]
        elif "@fields@" in data:
            for f in data["@fields@"]:
                if until and until == f:
                    break
                buf[len(buf) :] = self._get_bytes(data["@fields@"][f])

        return buf

    def _set_data(self, fields, value):
        data = self._data
        for f in fields.split('.'):
            if "@fields@" not in data:
                return
            data = data["@fields@"][f]

        if "@fields@" in data:
            raise Exception("%s is not leaf field" % fields)

        size = self.sizeof(fields)
        if "@buf@" not in data:
            data["@buf@"] = bytearray(self._padding * size)

        if type(value) in [int]:
            size_type = DATA_SIZE.get(size, 8)
            value = struct.pack(size_type, value)
            if size > len(value):
                value = value + self._padding * (size - len(value))
        elif type(value) in [str] and ':' in value:
            _,value = self._get_desc_data(value, 0)

        if data["@desc@"]:
            # data size cannot be changed
            if '@' in data["@desc@"]:
                raise Exception("%s is determined internally and cannot be changed!" % fields)
            size = min(size, len(value))
            data["@buf@"][:size] = value[:size]
        else:
            # data size is free
            data["@buf@"][:] = value
            self._fixup_()

    def _get_data(self, fields=''):
        fields = fields.split('.') if fields else []
        data = self._data
        for f in fields:
            if "@fields@" not in data or f not in data["@fields@"]:
                return None
            data = data["@fields@"][f]

        return self._get_bytes(data)

    def sizeof(self, fields=''):
        data = self._data
        buf = None
        fields = fields.split('.')
        for f in fields:
            if "@fields@" not in data:
                return None
            data = data["@fields@"][f]
        return self._sizeof_(data)

    def offset(self, fields):
        return self._offsetof_(fields)

    def to_file(self, bin_file):
        with open(bin_file, "wb") as fd:
            fd.write(self.data)

    @property
    def data(self):
        return self._get_data()

if __name__ == "__main__":
    if len(sys.argv) > 1:
        desc_file = sys.argv[1]
    else:
        desc_file = "var_store.json"

    with open(desc_file) as fd:
        var_store_info = json.load(fd, object_pairs_hook=OrderedDict)
        buf = DictStruct(var_store_info, 0x23)

        print("Description", buf["Description"])
        print("RootKey", buf["RootKey"])
        print("InitVec", buf["InitVec"])

        field = "FvHeader.BlockMap.0.NumBlocks"
        print(field, hex(buf.offset(field)), hex(buf.sizeof(field)), buf[field].hex())

        field = "VarStoreHeader"
        print(field, hex(buf.offset(field)), hex(buf.sizeof(field)), buf[field].hex())

        field = "VarStoreHeader.Signature"
        print(field, hex(buf.offset(field)), hex(buf.sizeof(field)), buf[field].hex())

        print()
        field = "Variables.0.#padding"
        print(field, hex(buf.offset(field)), hex(buf.sizeof(field)), buf[field].hex())
        field = "Variables.0.StartId"
        print(field, hex(buf.offset(field)), hex(buf.sizeof(field)), buf[field].hex())

        field = "Variables.1.#padding"
        print(field, hex(buf.offset(field)), hex(buf.sizeof(field)), buf[field].hex())
        field = "Variables.1.StartId"
        print(field, hex(buf.offset(field)), hex(buf.sizeof(field)), buf[field].hex())

        field = "Variables.2.#padding"
        print(field, hex(buf.offset(field)), hex(buf.sizeof(field)), buf[field].hex())
        field = "Variables.2.StartId"
        print(field, hex(buf.offset(field)), hex(buf.sizeof(field)), buf[field].hex())

        print()
        field = "FvHeader.Checksum"
        print(field, hex(buf.offset(field)), hex(buf.sizeof(field)), buf[field].hex())
        buf[field] = 0x10000-buf.checksum(buf["FvHeader"])
        print(field, hex(buf.offset(field)), hex(buf.sizeof(field)), buf[field].hex())

        # add more data
        print()
        field = "Variables.0.Data"
        print(field, hex(buf.offset(field)), hex(buf.sizeof(field)), buf[field].hex())
        buf[field] = b"\x01\x02\x03\x04"
        print(field, hex(buf.offset(field)), hex(buf.sizeof(field)), buf[field].hex())

        print()
        field = "Variables.0.#padding"
        print(field, hex(buf.offset(field)), hex(buf.sizeof(field)), buf[field].hex())
        field = "Variables.0.StartId"
        print(field, hex(buf.offset(field)), hex(buf.sizeof(field)), buf[field].hex())

        field = "Variables.1.#padding"
        print(field, hex(buf.offset(field)), hex(buf.sizeof(field)), buf[field].hex())
        field = "Variables.1.StartId"
        print(field, hex(buf.offset(field)), hex(buf.sizeof(field)), buf[field].hex())

        field = "Variables.2.#padding"
        print(field, hex(buf.offset(field)), hex(buf.sizeof(field)), buf[field].hex())
        field = "Variables.2.StartId"
        print(field, hex(buf.offset(field)), hex(buf.sizeof(field)), buf[field].hex())

        #
        # delete some data
        #
        print()
        field = "Variables.0.Data"
        print(field, hex(buf.offset(field)), hex(buf.sizeof(field)), buf[field].hex())
        buf[field] = "B:0x73"
        print(field, hex(buf.offset(field)), hex(buf.sizeof(field)), buf[field].hex())

        print()
        field = "Variables.0.#padding"
        print(field, hex(buf.offset(field)), hex(buf.sizeof(field)), buf[field].hex())
        field = "Variables.0.StartId"
        print(field, hex(buf.offset(field)), hex(buf.sizeof(field)), buf[field].hex())

        field = "Variables.1.#padding"
        print(field, hex(buf.offset(field)), hex(buf.sizeof(field)), buf[field].hex())
        field = "Variables.1.StartId"
        print(field, hex(buf.offset(field)), hex(buf.sizeof(field)), buf[field].hex())

        field = "Variables.2.#padding"
        print(field, hex(buf.offset(field)), hex(buf.sizeof(field)), buf[field].hex())
        field = "Variables.2.StartId"
        print(field, hex(buf.offset(field)), hex(buf.sizeof(field)), buf[field].hex())

        buf.to_file("test.fv")


