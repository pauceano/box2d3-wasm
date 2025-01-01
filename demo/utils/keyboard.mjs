const state = {
	initialized: false,
	down: {},
	pressed: {}
};

class Keyboard {
	static Init() {
		console.assert(!state.initialized, 'Keyboard already initialized');
		window.addEventListener('keydown', Keyboard.keyDown);
		window.addEventListener('keyup', Keyboard.keyUp);
		state.initialized = true;
	}

	static Destroy() {
		window.removeEventListener('keydown', Keyboard.keyDown);
		window.removeEventListener('keyup', Keyboard.keyUp);

		state.down = {};
		state.pressed = {};
		state.initialized = false;
	}

	static keyDown(event) {
		if (!state.down[event.key]) {
			state.pressed[event.key] = true;
		}
		state.down[event.key] = true;
	}

	static keyUp(event) {
		state.down[event.key] = false;
	}

	static IsDown(key) {
		return state.down[key];
	}

	static IsPressed(key) {
		return state.pressed[key];
	}

	static Update() {
		state.pressed = {};
	}
}

export default Keyboard;

export const Key = {
	Backspace: 'Backspace',
	Tab: 'Tab',
	Enter: 'Enter',
	Shift: 'Shift',
	Ctrl: 'Control',
	Alt: 'Alt',
	Pause: 'Pause',
	CapsLock: 'CapsLock',
	Escape: 'Escape',
	Space: ' ',
	PageUp: 'PageUp',
	PageDown: 'PageDown',
	End: 'End',
	Home: 'Home',
	ArrowLeft: 'ArrowLeft',
	ArrowUp: 'ArrowUp',
	ArrowRight: 'ArrowRight',
	ArrowDown: 'ArrowDown',
	PrintScreen: 'PrintScreen',
	Insert: 'Insert',
	Delete: 'Delete',
	Zero: '0',
	One: '1',
	Two: '2',
	Three: '3',
	Four: '4',
	Five: '5',
	Six: '6',
	Seven: '7',
	Eight: '8',
	Nine: '9',
	A: 'a',
	B: 'b',
	C: 'c',
	D: 'd',
	E: 'e',
	F: 'f',
	G: 'g',
	H: 'h',
	I: 'i',
	J: 'j',
	K: 'k',
	L: 'l',
	M: 'm',
	N: 'n',
	O: 'o',
	P: 'p',
	Q: 'q',
	R: 'r',
	S: 's',
	T: 't',
	U: 'u',
	V: 'v',
	W: 'w',
	X: 'x',
	Y: 'y',
	Z: 'z',
};
