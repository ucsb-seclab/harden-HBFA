# Title: Support for proxy server URI in HTTP Boot

# Status: Draft

# Document: UEFI specification Version 2.10

# License

SPDX-License-Identifier: CC-BY-4.0

# Submitter: [Saloni Kasbekar](mailto:saloni.kasbekar@intel.com), [Maciej Rabeda](mailto:maciej.rabeda@intel.com)

# Summary of change

Extension of section 24.7 explaining:
- HTTP boot scenario when proxy server is utilized.
- Changes to HTTP boot device path (proxy URI node).

Extension of section 29.6:
- New value (HttpMethodConnect) in EFI_HTTP_METHOD enum.

# Benefits of the change

Proposed changes:
- introduce support for HTTP CONNECT request.
- allow for better understanding of functionality being added to EDK2.

# Impact of the change

Additional content is added to the UEFI specification.
Numbering of sections 24.7.10-12 is shifted forward.

# Detailed description of the change

## (New section) 24.7.2.3 Use case with Proxy Server

No DHCP server
Role of DNS (proxy server IP resolution from Uri node)

## (Extension) 24.7.3.1 Device Path

(Uri() node for proxy server)

## (New chapter) 24.7.10 Concept of Message Exchange in HTTP Boot scenario (with Proxy Server)

Diagram:
- EFI HTTPBoot Client <-> Proxy Server
- EFI HTTPBoot Client <-> DNS
- Proxy Server <-> Endpoint Server

## (Extension) 29.6.6 EFI_HTTP_PROTOCOL.Request()
```
//*******************************************
// EFI_HTTP_METHOD
//*******************************************
typedef enum {
HttpMethodGet,
HttpMethodPost,
HttpMethodPatch,
HttpMethodOptions,
HttpMethodConnect,
HttpMethodHead,
HttpMethodPut,
HttpMethodDelete,
HttpMethodTrace,
HttpMethodMax
} EFI_HTTP_METHOD;
```