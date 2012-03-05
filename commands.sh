
# This buffer is for notes you don't want to save, and for Lisp evaluation.
# If you want to create a file, visit that file with C-x C-f,
# then enter the text in that file's own buffer.


#./VisualBoyAdvance ../../../pokemon-escape/roms/Pokemon\ Yellow\ \(U\)\ \[C\]\[\!\].gbc                                     


#./VisualBoyAdvance --recordmovie=./rlm.vbm ../../../pokemon-escape/roms/Pokemon\ Yellow\ \(U\)\ \[C\]\[\!\].gbc                                     

rm -f /home/r/proj/vba-restore/build/movie.vba

cd /home/r/proj/vba-restore/build
make install

/home/r/proj/vba-restore/build/artifacts/bin/VisualBoyAdvance --recordmovie=/home/r/proj/vba-restore/build/movie.vba /home/r/proj/pokemon-escape/roms/yellow.gbc                           

#.//VisualBoyAdvance --playmovie=/home/r/proj/vba-restore/build/movie.vba /home/r/proj/pokemon-escape/roms/Pokemon\ Yellow\ \(U\)\ \[C\]\[\!\].gbc                             

hexdump -C /home/r/proj/vba-restore/build/movie.vba
