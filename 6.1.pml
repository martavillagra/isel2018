ltl spec {
	[] (((state == 0) && button0) -> <> (state == 1)) &&
	[] (((state == 1) && button0) -> <> (state == 0)) 
}

bit button0;
bit button15;
byte state;
active proctype alarma_basic_fsm () {
	state = 0;
	do
	:: (state == 0) -> atomic {
		if
		:: button0 -> state = 1; button0 = 0
		fi
		}
	:: (state == 1) -> atomic {
		if
		:: button0 -> state = 0; button0 = 0
		:: button15 -> button15 = 0
		fi
		}
	od
}
active proctype entorno () {
	do
	::if
		:: button0  = 1
		:: (! button0) -> skip
	fi
	od
	do
	::if
		:: button15 = 1
		:: (! button15) -> skip
	fi	
	od
}