all_aimos:
	(mkdir -p bin)
	(cd graph_algos ; make aimos)
	(cd generate ; make aimos)

all:
	(mkdir -p bin)
	(cd graph_algos ; make not_aimos)
	(cd generate ; make not_aimos)