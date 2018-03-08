ltl spec {
	[] (((state == 0) && (codigo[0] == 0 && codigo[1] == 0 && codigo[2]== 0)) -> <> (state == 1)) &&
	[] ((state == 1) -> <> (state == 0)) 
}

#define timeout true