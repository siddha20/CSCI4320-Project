all_aimos:
	(cd graph_algos ; mkdir bin ; make aimos)
	(cd graph_gen ; mkdir bin ; make aimos)

all_not_aimos:
	(cd graph_algos ; mkdir bin ; make not_aimos)
	(cd graph_gen ; mkdir bin ; make not_aimos)