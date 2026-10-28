# Asset Content Fingerprints

ECOS Studio exposes `sha256_hex(std::string_view)`, a dependency-free SHA-256 digest over the exact input bytes. The lowercase 64-character digest can key import caches and detect source changes; it is not an asset ID and must not replace schema/version checks or dependency validation.

Known SHA-256 test vectors (empty input and `abc`) are checked in CI. Large-file streaming and persistent cache metadata are future extensions; this API currently hashes an in-memory byte view.
