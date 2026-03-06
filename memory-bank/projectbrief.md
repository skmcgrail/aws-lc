# Overview

AWS-LC provides libcrypto and libssl implementations that are mostly API compatible with OpenSSL 1.1.1. I would like to explore seperating out the AWS-LC types from the OpenSSL types to make a clearer boundary between the "compatability layer" functions and types. This way we can later come back in a future project and implement AWS-LC as an OpenSSL 3.x provider that be be plugged into OpenSSL.

# Example from WolfSSL

WolfSSL defines it's functions like:
```
const WOLFSSL_EVP_MD* wolfSSL_EVP_sha256(void);
```

Where they prefix types and functions with `WOLFSSL`/`wolfSSL` as to not conflict with OpenSSL.
The `openssl/*` headers then have various defines like:
```
#define EVP_sha256        wolfSSL_EVP_sha256
```

Providing the compatability mapping. Do things will allow our library to leverage the necessary OpenSSL 3.x provider APIs and properly integrate without conflicts.
