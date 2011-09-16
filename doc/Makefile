.PHONY: all clean

all: doc.dvi

%.pdf: %.ps
	ps2pdf $< $@

%.ps: %.dvi
	dvips -o $@ $<

%.dvi: %.tex
	latex $<

clean:
	rm -f *.pdf *.ps *.aux *.toc *.dvi *.log
