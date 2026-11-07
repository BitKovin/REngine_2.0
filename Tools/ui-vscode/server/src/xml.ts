// A forgiving XML scanner: it keeps offsets of every name and value, and never throws on half-typed markup,
// because completion has to work in the middle of an edit.

export interface Attr {
  name: string;
  nameStart: number;
  nameEnd: number;
  hasValue: boolean;
  value: string;
  valueStart: number;      // first character inside the quotes
  valueEnd: number;        // offset of the closing quote (or end of the text if it is missing)
  quoted: boolean;
}

export interface El {
  name: string;
  start: number;           // offset of '<'
  nameStart: number;
  nameEnd: number;
  attrs: Attr[];
  tagEnd: number;          // offset of '>' (or where scanning stopped when the tag is not finished)
  terminated: boolean;     // the open tag has its '>'
  selfClosing: boolean;
  closeStart: number;      // offset of '</' of the matching close tag, -1 if none
  closeEnd: number;
  parent: El | null;
  children: El[];
}

export interface Problem {
  start: number;
  end: number;
  message: string;
}

export interface Scan {
  roots: El[];
  all: El[];
  problems: Problem[];
}

const NAME_CHAR = /[\w.:-]/;

export function scanXml(text: string): Scan {
  const roots: El[] = [];
  const all: El[] = [];
  const problems: Problem[] = [];
  const stack: El[] = [];
  const n = text.length;
  let i = 0;

  const skipWs = () => { while (i < n && /\s/.test(text[i])) i++; };

  while (i < n) {
    const lt = text.indexOf('<', i);
    if (lt < 0) break;
    i = lt;
    if (text.startsWith('<!--', i)) {
      const e = text.indexOf('-->', i + 4);
      if (e < 0) { problems.push({ start: i, end: n, message: 'Unterminated comment' }); i = n; } else i = e + 3;
      continue;
    }
    if (text.startsWith('<![CDATA[', i)) {
      const e = text.indexOf(']]>', i);
      i = e < 0 ? n : e + 3;
      continue;
    }
    if (text.startsWith('<?', i) || text.startsWith('<!', i)) {
      const e = text.indexOf('>', i);
      i = e < 0 ? n : e + 1;
      continue;
    }
    if (text.startsWith('</', i)) {
      let j = i + 2;
      const ns = j;
      while (j < n && NAME_CHAR.test(text[j])) j++;
      const name = text.slice(ns, j);
      while (j < n && /\s/.test(text[j])) j++;
      const closed = text[j] === '>';
      const end = closed ? j + 1 : j;
      // find the open element this closes
      let k = stack.length - 1;
      while (k >= 0 && stack[k].name !== name) k--;
      if (k < 0) {
        problems.push({ start: i, end: Math.max(end, i + 2), message: `Closing tag </${name}> has no matching opening tag` });
      } else {
        for (let m = stack.length - 1; m > k; m--) {
          const open = stack[m];
          problems.push({ start: open.nameStart, end: open.nameEnd, message: `<${open.name}> is not closed` });
        }
        stack[k].closeStart = i;
        stack[k].closeEnd = end;
        stack.length = k;
      }
      if (!closed) problems.push({ start: i, end: Math.max(end, i + 2), message: 'Closing tag is missing ">"' });
      i = end;
      continue;
    }

    // opening tag
    const el: El = {
      name: '', start: i, nameStart: i + 1, nameEnd: i + 1, attrs: [], tagEnd: i + 1, terminated: false, selfClosing: false,
      closeStart: -1, closeEnd: -1, parent: stack.length ? stack[stack.length - 1] : null, children: [],
    };
    i++;
    while (i < n && NAME_CHAR.test(text[i])) i++;
    el.name = text.slice(el.nameStart, i);
    el.nameEnd = i;

    for (;;) {
      skipWs();
      if (i >= n) { el.tagEnd = n; break; }
      const c = text[i];
      if (c === '>') { el.terminated = true; el.tagEnd = i; i++; break; }
      if (c === '/' && text[i + 1] === '>') { el.terminated = true; el.selfClosing = true; el.tagEnd = i; i += 2; break; }
      if (c === '<') { el.tagEnd = i; break; }                    // the next tag starts: this one was never finished
      if (c === '/') { i++; continue; }
      if (c === '"' || c === "'") {                               // stray value without a name
        const q = text.indexOf(c, i + 1);
        i = q < 0 ? n : q + 1;
        continue;
      }
      const a: Attr = { name: '', nameStart: i, nameEnd: i, hasValue: false, value: '', valueStart: i, valueEnd: i, quoted: false };
      while (i < n && !/[\s=/>"'<]/.test(text[i])) i++;
      a.name = text.slice(a.nameStart, i);
      a.nameEnd = i;
      let j = i;
      while (j < n && /\s/.test(text[j])) j++;
      if (text[j] === '=') {
        a.hasValue = true;
        j++;
        while (j < n && /\s/.test(text[j])) j++;
        const q = text[j];
        if (q === '"' || q === "'") {
          a.quoted = true;
          a.valueStart = j + 1;
          const e = text.indexOf(q, j + 1);
          a.valueEnd = e < 0 ? n : e;
          i = e < 0 ? n : e + 1;
        } else {
          a.valueStart = j;
          while (j < n && !/[\s>]/.test(text[j]) && !(text[j] === '/' && text[j + 1] === '>')) j++;
          a.valueEnd = j;
          i = j;
        }
        a.value = text.slice(a.valueStart, a.valueEnd);
      }
      if (a.name === '' && !a.hasValue) { i++; continue; }
      el.attrs.push(a);
    }

    all.push(el);
    if (el.parent) el.parent.children.push(el); else roots.push(el);
    if (!el.terminated) {
      problems.push({ start: el.start, end: Math.max(el.nameEnd, el.start + 1), message: `Tag <${el.name}> is not finished (missing ">")` });
    } else if (!el.selfClosing) {
      stack.push(el);
    }
  }
  for (const open of stack) problems.push({ start: open.nameStart, end: open.nameEnd, message: `<${open.name}> is not closed` });
  return { roots, all, problems };
}

export type XmlContext =
  | { kind: 'tagName'; el: El; prefix: string; start: number }
  | { kind: 'closeTag'; start: number; prefix: string }
  | { kind: 'attrName'; el: El; prefix: string; start: number; end: number; hasValue: boolean }
  | { kind: 'attrValue'; el: El; attr: Attr }
  | { kind: 'text' };

// What is the cursor in? Works on unfinished markup.
export function contextAt(scan: Scan, text: string, offset: number): XmlContext {
  const before = text.slice(0, offset);
  const close = /<\/([\w.:-]*)$/.exec(before);
  if (close) return { kind: 'closeTag', start: offset - close[1].length, prefix: close[1] };

  for (const el of scan.all) {
    if (offset <= el.start) continue;
    const inTag = el.terminated ? offset <= el.tagEnd : offset <= el.tagEnd;
    if (!inTag) continue;
    if (offset >= el.nameStart && offset <= el.nameEnd) return { kind: 'tagName', el, prefix: text.slice(el.nameStart, offset), start: el.nameStart };
    for (const a of el.attrs) {
      if (a.hasValue && offset >= a.valueStart && offset <= a.valueEnd && (a.quoted || offset <= a.valueEnd)) {
        return { kind: 'attrValue', el, attr: a };
      }
      if (offset >= a.nameStart && offset <= a.nameEnd) {
        return { kind: 'attrName', el, prefix: text.slice(a.nameStart, offset), start: a.nameStart, end: a.nameEnd, hasValue: a.hasValue };
      }
    }
    if (offset > el.nameEnd) return { kind: 'attrName', el, prefix: '', start: offset, end: offset, hasValue: false };
  }
  return { kind: 'text' };
}

// The innermost element whose content contains the offset.
export function enclosingElement(scan: Scan, offset: number): El | null {
  let best: El | null = null;
  for (const el of scan.all) {
    if (!el.terminated || el.selfClosing || offset <= el.tagEnd) continue;
    if (el.closeStart >= 0 && offset > el.closeStart) continue;
    if (!best || el.start > best.start) best = el;
  }
  return best;
}
