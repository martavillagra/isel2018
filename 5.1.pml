ltl spec {
	[] (((state == 0) && button) -> <> (state == 1)) &&
	[] ((state == 1) -> <> (state == 0)) 
}

bit button;
#define timeout true
byte state;

active proctype apagado_temp_fsm () {
	state = 0;
	do
	:: (state == 0) -> atomic {
		if
		:: button -> state = 1; button = 0
		fi
		}
	:: (state == 1) -> atomic {
		if
		:: button ->  state=0; button = 0
		:: timeout ->   state = 0
		fi
		}
	od
}
active proctype entorno () {
	do
	:: button = 1
	:: (! button) -> skip
	od
}