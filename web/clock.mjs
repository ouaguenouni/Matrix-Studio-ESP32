export const CLOCK_THEMES = {
  classic: {background: '#000000', foreground: '#ffffff', accent: '#ffffff', blink: true},
  amber: {background: '#2a1408', foreground: '#ffb040', accent: '#ffb040', blink: false},
  neon: {background: '#000000', foreground: '#00ffff', accent: '#ff00ff', blink: false},
};

export const DIGITS = {
  '0': ['111', '101', '101', '101', '111'],
  '1': ['010', '110', '010', '010', '111'],
  '2': ['111', '001', '111', '100', '111'],
  '3': ['111', '001', '111', '001', '111'],
  '4': ['101', '101', '111', '001', '001'],
  '5': ['111', '100', '111', '001', '111'],
  '6': ['111', '100', '111', '101', '111'],
  '7': ['111', '001', '001', '001', '001'],
  '8': ['111', '101', '111', '101', '111'],
  '9': ['111', '101', '111', '001', '111'],
  ':': ['0', '1', '0', '1', '0'],
  '-': ['000', '000', '111', '000', '000'],
  ' ': ['000', '000', '000', '000', '000'],
};

export function clockFace(date) {
  const second = date.getSeconds();
  return {
    text: `${String(date.getHours()).padStart(2, '0')}:${String(date.getMinutes()).padStart(2, '0')}`,
    showColon: second % 2 === 0,
    second,
    bar: Math.max(1, Math.round(((second + 1) / 60) * 64)),
  };
}

export function digitWidth(text, scale) {
  let width = 0;
  for (const char of text) {
    const rows = DIGITS[char] || DIGITS[' '];
    width += rows[0].length * scale + scale;
  }
  return text.length ? width - scale : 0;
}
