with import <nixpkgs> {};
mkShell.override { stdenv = llvmPackages_12.stdenv; } {
    buildInputs = [
        llvmPackages_12.lld
        shellcheck
        valgrind
    ];
    shellHook = ''
        . .shellhook
    '';
}
