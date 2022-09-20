# edk2-staging - HTTP Proxy branch

Code-first branch for HTTP proxy support (mainly for HTTP boot).

Changes have impact on UEFI specification (ProxyServer URI device path node in HTTP boot device path).

```
//....../Mac(...)[/Vlan(...)][/Wi-Fi(...)]/IPv6(...)[/Dns(...)]/Uri(ProxyServer)/Uri(EndPointServer/FilePath)
```

Relevant BugZilla links:
- https://bugzilla.tianocore.org/show_bug.cgi?id=3951
- https://bugzilla.tianocore.org/show_bug.cgi?id=4052

## Scope of required work
1. MdeModulePkg/UefiBootManagerLib - adjust matching of HTTP boot device path
2. NetworkPkg/HttpDxe - support for HTTP CONNECT requests, TLS adjustments
3. NetworkPkg/HttpBootDxe - device path parsing, establishing connection with proxy
