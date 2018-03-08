ltl spec {
	[] (((state == OFF0) && button) -> <> (state == OFF1)) &&
	[] (((state == OFF1) && button) -> <> (state == OFF2)) &&
	[] (((state == OFF2) && button) -> <> (state == OFF3)) &&
	[] ((state == OFF3) -> <> (state == ON0)) &&
	[] (((state == ON0) && button) -> <> (state == ON1)) &&
	[] (((state == ON1) && button) -> <> (state == ON2)) &&
	[] (((state == ON2) && button) -> <> (state == ON3)) &&
	[] ((state == ON3) -> <> (state == OFF0)) 
}

mtype={OFF0, OFF1, OFF2, OFF3, ON0, ON1, ON2, ON3}