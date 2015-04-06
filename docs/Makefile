all: index.html dan64_v1-manual.pdf

index.html: index.markdown
	pandoc -T "DAN64: an AVR based 8-bit microcomputer" -s -o index.html index.markdown

dan64_v1-manual.pdf: manual.markdown dan64_v1-schematics.pdf
	pandoc -M classoption=oneside,onecolumn -H before.latex -T "DAN64: an AVR based 8-bit microcomputer" -s -o dan64_v1-manual.pdf manual.markdown
clean:
	rm -f index.html dan64_v1-manual.pdf

