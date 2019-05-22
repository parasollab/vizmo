LTX=latex
DVI=dvips
PDF=ps2pdf
DOC=refman
IDX=makeindex

refman:	$(DOC).tex algo.tex cont.tex view.tex gralg.tex
	cat /dev/null > $(DOC).ind
	$(LTX) $(DOC).tex 
	$(LTX) $(DOC).tex >& /dev/null
	$(IDX) $(DOC) >& $(DOC).idxlog
	$(LTX) $(DOC).tex >& $(DOC).ltxlog
	mv $(DOC).ind index.tex
	$(DVI) $(DOC).dvi -o $(DOC).ps >& $(DOC).dvilog
	$(PDF) $(DOC).ps > $(DOC).pdf

clean:
	rm -f $(DOC).{aux,dvi,ps,ltxlog,dvilog,log,idx,idxlog,ilg,ind}

examples: cont_ex view_ex algo_ex gralg_ex
	./make_examples
