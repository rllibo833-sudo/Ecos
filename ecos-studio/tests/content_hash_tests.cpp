#include "ecos_studio/content_hash.hpp"

#include <iostream>
#include <string>

int main() {
    using ecos::studio::sha256_hex;
    int failures=0;
    auto check=[&](bool ok,const char* label) {
        if (!ok) { std::cerr << "FAIL: " << label << '\n'; ++failures; }
    };
    check(sha256_hex("")=="e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
          "matches SHA-256 empty input test vector");
    check(sha256_hex("abc")=="ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
          "matches SHA-256 abc test vector");
    check(sha256_hex("abc")==sha256_hex(std::string("abc")),
          "hashes identical bytes deterministically");
    check(sha256_hex("abc")!=sha256_hex("abd"),
          "content changes alter the digest");
    if (failures) return 1;
    std::cout << "ECOS Studio SHA-256 tests passed.\n";
    return 0;
}
