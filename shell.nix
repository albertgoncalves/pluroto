with import <nixpkgs> {};
mkShell.override { stdenv = llvmPackages_12.stdenv; } {
    buildInputs = [
        mold
        shellcheck
        valgrind
    ];
    shellHook = ''
        . .shellhook
    '';
}
