with import <nixpkgs> {};
stdenv.mkDerivation {
  src = ./.;
  name = "ff-notext";
  enableParallelBuilding = true;

  nativeBuildInputs = [ pkg-config ];
  buildInputs = [ tesseract5 ];

  installPhase = ''
    make install PREFIX=$out
  '';
}
