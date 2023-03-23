default: tokenizer


tokenizer: token.c
	gcc -o tokenizer -I/home/dmg/git.w/srcML/src/libsrcml -L/home/dmg/git.w/srcML/build/bin token.c -lsrcml
