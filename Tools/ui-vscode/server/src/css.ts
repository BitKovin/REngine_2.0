// Tolerant parser for the engine's CSS flavour: rules, declarations, @import, @keyframes, custom properties.
import { Problem } from './xml';

export interface Decl {
  name: string;
  nameStart: number;
  nameEnd: number;
  value: string;
  valueStart: number;
  valueEnd: number;
}

export interface Rule {
  selector: string;
  selectorStart: number;
  selectorEnd: number;
  bodyStart: number;          // after '{'
  bodyEnd: number;            // offset of '}' (or end of text)
  decls: Decl[];
  inKeyframes: boolean;
}

export interface Sheet {
  rules: Rule[];
  imports: { src: string; start: number; end: number }[];
  keyframes: { name: string; start: number; end: number }[];
  customProps: Map<string, { offset: number; value: string }>;
  classes: Map<string, number>;      // class name -> offset of first selector use
  ids: Map<string, number>;
  problems: Problem[];
  clean: string;                     // text with comments blanked out (same offsets)
}

export const KNOWN_AT_RULES = ['@import', '@keyframes'];

export function blankComments(text: string): string {
  let out = '';
  let i = 0;
  while (i < text.length) {
    if (text[i] === '/' && text[i + 1] === '*') {
      const e = text.indexOf('*/', i + 2);
      const end = e < 0 ? text.length : e + 2;
      out += text.slice(i, end).replace(/[^\n]/g, ' ');
      i = end;
    } else if (text[i] === '"' || text[i] === "'") {
      const q = text[i];
      let j = i + 1;
      while (j < text.length && text[j] !== q && text[j] !== '\n') j++;
      out += text.slice(i, Math.min(j + 1, text.length));
      i = Math.min(j + 1, text.length);
    } else {
      out += text[i++];
    }
  }
  return out;
}

// Splits a declaration block "a: b; c: d" at top-level semicolons (not inside () or quotes).
export function parseDecls(clean: string, start: number, end: number): Decl[] {
  const decls: Decl[] = [];
  let i = start;
  while (i < end) {
    let j = i, depth = 0, q = '';
    while (j < end) {
      const c = clean[j];
      if (q) { if (c === q) q = ''; }
      else if (c === '"' || c === "'") q = c;
      else if (c === '(') depth++;
      else if (c === ')') depth = Math.max(0, depth - 1);
      else if (c === ';' && depth === 0) break;
      j++;
    }
    const seg = clean.slice(i, j);
    const colon = seg.indexOf(':');
    if (seg.trim().length > 0) {
      if (colon < 0) {
        const lead = seg.length - seg.trimStart().length;
        decls.push({ name: seg.trim(), nameStart: i + lead, nameEnd: i + lead + seg.trim().length, value: '', valueStart: j, valueEnd: j });
      } else {
        const rawName = seg.slice(0, colon);
        const lead = rawName.length - rawName.trimStart().length;
        const value = seg.slice(colon + 1);
        const vlead = value.length - value.trimStart().length;
        decls.push({
          name: rawName.trim(), nameStart: i + lead, nameEnd: i + lead + rawName.trim().length,
          value: value.trim(), valueStart: i + colon + 1 + vlead, valueEnd: i + colon + 1 + vlead + value.trim().length,
        });
      }
    }
    i = j + 1;
  }
  return decls;
}

export function parseCss(text: string): Sheet {
  const clean = blankComments(text);
  const sheet: Sheet = {
    rules: [], imports: [], keyframes: [], customProps: new Map(), classes: new Map(), ids: new Map(), problems: [], clean,
  };
  const n = clean.length;

  const addSelectorNames = (sel: string, base: number) => {
    for (const m of sel.matchAll(/\.(-?[_a-zA-Z][\w-]*)/g)) if (!sheet.classes.has(m[1])) sheet.classes.set(m[1], base + m.index!);
    for (const m of sel.matchAll(/#(-?[_a-zA-Z][\w-]*)/g)) if (!sheet.ids.has(m[1])) sheet.ids.set(m[1], base + m.index!);
  };

  // parses rules from i until the end or a closing brace at this level; returns the new offset
  const parseLevel = (i: number, inKeyframes: boolean): number => {
    while (i < n) {
      while (i < n && /\s/.test(clean[i])) i++;
      if (i >= n) break;
      if (clean[i] === '}') return i;
      if (clean[i] === ';') { i++; continue; }

      if (clean[i] === '@') {
        const m = /^@[\w-]+/.exec(clean.slice(i, i + 40));
        const kw = m ? m[0] : '@';
        const hdrEnd = (() => { let j = i; while (j < n && clean[j] !== '{' && clean[j] !== ';') j++; return j; })();
        const header = clean.slice(i, hdrEnd);
        if (kw === '@import') {
          const s = /["']([^"']*)["']/.exec(header);
          if (s) sheet.imports.push({ src: s[1], start: i + s.index + 1, end: i + s.index + 1 + s[1].length });
          else sheet.problems.push({ start: i, end: i + kw.length, message: '@import needs a quoted path' });
          i = hdrEnd + 1;
        } else if (kw === '@keyframes') {
          const nm = /^@keyframes\s+([\w-]+)/.exec(header);
          if (nm) sheet.keyframes.push({ name: nm[1], start: i + header.indexOf(nm[1], 10), end: i + header.indexOf(nm[1], 10) + nm[1].length });
          else sheet.problems.push({ start: i, end: i + kw.length, message: '@keyframes needs a name' });
          if (clean[hdrEnd] === '{') {
            const stop = parseLevel(hdrEnd + 1, true);
            i = stop + 1;
          } else i = hdrEnd + 1;
        } else {
          sheet.problems.push({ start: i, end: i + kw.length, message: `Unknown at-rule ${kw}` });
          if (clean[hdrEnd] === '{') {
            let depth = 1, j = hdrEnd + 1;
            while (j < n && depth > 0) { if (clean[j] === '{') depth++; else if (clean[j] === '}') depth--; j++; }
            i = j;
          } else i = hdrEnd + 1;
        }
        continue;
      }

      // style rule: selector { declarations }
      const selStart = i;
      while (i < n && clean[i] !== '{' && clean[i] !== '}' && clean[i] !== ';') i++;
      const selRaw = clean.slice(selStart, i);
      if (i >= n || clean[i] !== '{') {
        sheet.problems.push({ start: selStart, end: Math.max(selStart + selRaw.trimEnd().length, selStart + 1), message: 'Selector has no "{ ... }" block' });
        if (i < n && clean[i] === ';') i++;
        continue;
      }
      const bodyStart = i + 1;
      let j = bodyStart;
      while (j < n && clean[j] !== '}') {
        if (clean[j] === '{') {
          sheet.problems.push({ start: j, end: j + 1, message: 'Nested blocks are not supported' });
          let depth = 1; j++;
          while (j < n && depth > 0) { if (clean[j] === '{') depth++; else if (clean[j] === '}') depth--; j++; }
          continue;
        }
        j++;
      }
      if (j >= n) sheet.problems.push({ start: selStart, end: selStart + Math.max(1, selRaw.trimEnd().length), message: 'Missing closing "}"' });
      const decls = parseDecls(clean, bodyStart, j);
      const lead = selRaw.length - selRaw.trimStart().length;
      const selector = selRaw.trim();
      const rule: Rule = {
        selector, selectorStart: selStart + lead, selectorEnd: selStart + lead + selector.length,
        bodyStart, bodyEnd: j, decls, inKeyframes,
      };
      sheet.rules.push(rule);
      if (!inKeyframes) addSelectorNames(selector, rule.selectorStart);
      for (const d of decls) {
        if (d.name.startsWith('--') && !sheet.customProps.has(d.name)) sheet.customProps.set(d.name, { offset: d.nameStart, value: d.value });
      }
      i = j + 1;
    }
    return i;
  };

  let i = 0;
  while (i < n) {
    i = parseLevel(i, false);
    if (i < n && clean[i] === '}') { sheet.problems.push({ start: i, end: i + 1, message: 'Unexpected "}"' }); i++; }
  }
  return sheet;
}

export interface ColorHit {
  start: number;
  end: number;
  r: number; g: number; b: number; a: number;     // 0..1
}

// #rgb #rrggbb #rrggbbaa rgb() rgba()
export function findColors(text: string, from = 0, to = text.length): ColorHit[] {
  const out: ColorHit[] = [];
  const s = text.slice(from, to);
  for (const m of s.matchAll(/#([0-9a-fA-F]{8}|[0-9a-fA-F]{6}|[0-9a-fA-F]{4}|[0-9a-fA-F]{3})(?![\w-])/g)) {
    const h = m[1];
    const full = h.length <= 4 ? h.split('').map((c) => c + c).join('') : h;
    const v = (k: number) => parseInt(full.slice(k * 2, k * 2 + 2), 16) / 255;
    out.push({ start: from + m.index!, end: from + m.index! + m[0].length, r: v(0), g: v(1), b: v(2), a: full.length === 8 ? v(3) : 1 });
  }
  for (const m of s.matchAll(/rgba?\(\s*([\d.]+)\s*,\s*([\d.]+)\s*,\s*([\d.]+)\s*(?:,\s*([\d.]+)\s*)?\)/g)) {
    out.push({
      start: from + m.index!, end: from + m.index! + m[0].length,
      r: Math.min(255, +m[1]) / 255, g: Math.min(255, +m[2]) / 255, b: Math.min(255, +m[3]) / 255, a: m[4] === undefined ? 1 : Math.min(1, +m[4]),
    });
  }
  return out;
}
