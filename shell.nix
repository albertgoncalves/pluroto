with import <nixpkgs> {};
mkShell.override { stdenv = llvmPackages_14.stdenv; } {
    buildInputs = [
        mold
        shellcheck
        valgrind
    ];
    shellHook = ''
        . .shellhook
    '';
}
