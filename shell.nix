with import <nixpkgs> {};
mkShell.override { stdenv = llvmPackages_11.stdenv; } {
    buildInputs = [
        shellcheck
        valgrind
    ];
    shellHook = ''
        . .shellhook
    '';
}
