This branch was used to add new EC/Bignum/TLS APIs in CryptoPkg, this new API can be used by other modules. For example, Wi-Fi supplicant driver can use them to support WPA3 features.
The branch owner:
Heng Luo < heng.luo@intel.com >

## Feature Introduction
WI-FI module of BIOS need to support WPA3, which is the newest WI-FI security protocols. WPA3 adds new features to simplify WI-FI security, enable more robust authentication, deliver increased cryptographic strength for highly sensitive data markets, and maintain resiliency of mission critical networks.

WPA3-Personal brings better protections to individual users by providing more robust password-based authentication, even when users choose passwords that fall short of typical complexity recommendations. This capability is enabled through Simultaneous Authentication of Equals (SAE). The technology is resistant to offline dictionary attacks where an adversary attempts to determine a network password by trying possible passwords without further network interaction.

WPA3-Enterprise builds upon the foundation of WPA2-Enterprise with the additional requirement of using Protected Management Frames on all WPA3 connections. WPA3-Enterprise with SuiteB-192 mode uses Extensible Authentication Protocol – Transport Layer Security (EAP-TLS).

Wi‑Fi CERTIFIED Enhanced Open™ is a Wi-Fi Alliance certification that preserves the convenience open networks offer while reducing some of the risks associated with accessing an unsecured network. Wi-Fi Enhanced Open™ networks provide unauthenticated data encryption to users, an improvement over traditional open networks with no protections at all. These protections are transparent to the user. Based on Opportunistic Wireless Encryption (OWE) defined in the Internet Engineering Task Force (IETF) RFC8110 specification and the Wi‑Fi Alliance Opportunistic Wireless Encryption Specification, Wi-Fi Enhanced Open benefits users by providing data encryption that maintains the ease of use of open networks, and benefits network providers because there are no public passphrases to maintain, share, or manage.

Both SAE and OWE (which are part of WPA3) require big numbers and Elliptic Curve calculations in order to implement the authentication mechanisms. Since CryptoPkg is already including a major part of the required openssl crypto, if we can support related API in CryptoPkg, we will be able to avoid duplicate compilation of openssl and save overall SPI size.

EAP-TLS will use existing TLS APIs to implement the authentication, but current TLS APIs are not enough, need ot add some new TLS APIs in CryptoPkg.

Reference link:
[Discover Wi-Fi Security](https://www.wi-fi.org/discover-wi-fi/security)
[WPA3 specification](https://www.wi-fi.org/file/wpa3-specification)

#### Related Protocols
The following internal protocols are related to new APIs of CryptoPkg:
```
EDK II Crypto Protocol
```

#### Related Modules
The following modules are related to new APIs of CryptoPkg:
```
CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf
CryptoPkg/Library/BaseCryptLib/PeiCryptLib.inf
CryptoPkg/Library/BaseCryptLib/SmmCryptLib.inf
CryptoPkg/Library/BaseCryptLibNull/BaseCryptLibNull.inf
```

## Timeline
| Time | Event | Related Module |
|:----:|:-----:|:--------------:|
| 2022.5 | Initial implementation for new APIs of CryptoPkg. | CryptoDxe, CryptoSmm, BaseCryptLib, TlsLib |
|...|...|...|

## Feature Verification
There APIs are used by Wi-Fi Supplicant deriver, and the features are verified via Wi-Fi WPA3 test cases.

|Test Case| Test Result|
|:------------:|:-------:|
|WPA3 Personal(SAE) | Pass |
|Wi‑Fi CERTIFIED Enhanced Open (OWE) | Pass |
|WPA3 Enterprise(SuiteB-192) | Pass |
