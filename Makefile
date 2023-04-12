all:
	(cd graph_algos; make aimos)
	(cd graph_gen; make aimos)
all not_aimos:
	(cd graph_algos; make not_aimos)
	(cd graph_gen; make not_aimos)