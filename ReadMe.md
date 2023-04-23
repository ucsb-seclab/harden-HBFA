# OpenSSL 3.0

This branch will be used to develop the OpenSSL 3.0 to replace OpenSSL 1.1 for EDKII CryptoPkg.

## Problem Statement
According to https://www.OpenSSL.org/policies/releasestrat.html, OpenSSL Version 1.1.1 will be supported until 2023-09-11 (LTS).

The current size-reduced solution https://github.com/tianocore/edk2-staging/tree/OpenSSL11_EOL needs to submodule a tianocore specific openssl (https://github.com/tianocore/openssl). As such, it can only be used for edk2-staging.

The purpose of this branch is to submodule the original openssl (https://github.com/openssl/openssl). As such, it can be merged to edk2-master with the known size limitation.
