with import <nixpkgs> {};
mkShell.override { stdenv = llvmPackages_12.stdenv; } {
    buildInputs = [
        lld_12
        shellcheck
        valgrind
    ];
    shellHook = ''
        . .shellhook
    '';
}
