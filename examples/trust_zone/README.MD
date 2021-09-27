#TrustZone example with non-secure and secure zones.

##Project doesn't work right now.

The example contains two different projects.
The non_secure_zone and the secure_zone.

The non_secure_zone is built non-secure and can access secure functions
through non-secure callabacks that are located in secure memory.

The secure_zone is built safe by the use of the the Arm®v8‑M Security Extension.
-mcmse
