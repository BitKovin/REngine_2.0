import {
  CompletionItem, CompletionItemKind, Color, ColorInformation, ColorPresentation, Diagnostic, DiagnosticSeverity,
  DocumentSymbol, Hover, InsertTextFormat, Location, MarkupKind, SymbolKind, TextEdit,
} from 'vscode-languageserver';
import { TextDocument } from 'vscode-languageserver-textdocument';
import { blankComments, findColors, parseCss, parseDecls, Sheet } from './css';
import { CssProp, UiDb } from './db';
import { gameRootOf, Project, resolveGamePath } from './project';
import { URI } from 'vscode-uri';
import * as path from 'path';

const TRIGGER = { command: 'editor.action.triggerSuggest', title: 'Suggest' };

export type CssCtx =
  | { kind: 'none' }
  | { kind: 'atRule'; prefix: string; start: number }
  | { kind: 'importPath'; prefix: string; start: number }
  | { kind: 'selector'; sub: 'class' | 'id' | 'pseudo' | 'type' | 'keyframe'; prefix: string; start: number }
  | { kind: 'property'; prefix: string; start: number }
  | { kind: 'value'; property: string; prefix: string; start: number }
  | { kind: 'varName'; prefix: string; start: number };

// What is the cursor in? `clean` has comments blanked. With declarationsOnly the text is a bare "a: b; c: d" list
// (the style="" attribute of the markup).
export function cssContextAt(clean: string, offset: number, declarationsOnly = false): CssCtx {
  const stack: string[] = declarationsOnly ? ['{'] : [];
  let seg = 0;
  let quote = '';
  let quoteAt = -1;
  for (let i = 0; i < offset; i++) {
    const c = clean[i];
    if (quote) { if (c === quote || c === '\n') quote = ''; continue; }
    if (c === '"' || c === "'") { quote = c; quoteAt = i; continue; }
    if (c === '{') { stack.push(clean.slice(seg, i).trim()); seg = i + 1; }
    else if (c === '}') { stack.pop(); seg = i + 1; }
    else if (c === ';' && (stack.length > 0 || true)) seg = i + 1;
  }
  const text = clean.slice(seg, offset);
  if (quote) {
    if (/@import\s+["'][^"']*$/.test(text)) return { kind: 'importPath', prefix: clean.slice(quoteAt + 1, offset), start: quoteAt + 1 };
    return { kind: 'none' };
  }
  const top = stack.length ? stack[stack.length - 1] : null;
  const inKeyframes = top !== null && /^@keyframes\b/.test(top);

  if (top === null || inKeyframes) {
    const at = /^\s*@([\w-]*)$/.exec(text);
    if (at) return { kind: 'atRule', prefix: '@' + at[1], start: offset - at[1].length - 1 };
    if (inKeyframes) {
      const w = /[\w.%-]*$/.exec(text)![0];
      return { kind: 'selector', sub: 'keyframe', prefix: w, start: offset - w.length };
    }
    let m: RegExpExecArray | null;
    if ((m = /::?([\w-]*)$/.exec(text))) return { kind: 'selector', sub: 'pseudo', prefix: m[1], start: offset - m[1].length };
    if ((m = /\.([\w-]*)$/.exec(text))) return { kind: 'selector', sub: 'class', prefix: m[1], start: offset - m[1].length };
    if ((m = /#([\w-]*)$/.exec(text))) return { kind: 'selector', sub: 'id', prefix: m[1], start: offset - m[1].length };
    m = /[\w-]*$/.exec(text)!;
    return { kind: 'selector', sub: 'type', prefix: m[0], start: offset - m[0].length };
  }

  // inside a declaration block
  const colon = text.indexOf(':');
  if (colon < 0) {
    const w = /[\w-]*$/.exec(text)![0];
    return { kind: 'property', prefix: w, start: offset - w.length };
  }
  const property = text.slice(0, colon).trim();
  const value = text.slice(colon + 1);
  const v = /var\(\s*([\w-]*)$/.exec(value);
  if (v) return { kind: 'varName', prefix: v[1], start: offset - v[1].length };
  const w = /[\w#.-]*$/.exec(value)![0];
  return { kind: 'value', property, prefix: w, start: offset - w.length };
}

const NAMED_FUNCTIONS: { label: string; insert: string; detail: string }[] = [
  { label: 'var()', insert: 'var(--$1)', detail: 'Use a custom property (theme token)' },
  { label: 'rgb()', insert: 'rgb(${1:255}, ${2:255}, ${3:255})', detail: 'Colour from red, green, blue (0-255)' },
  { label: 'rgba()', insert: 'rgba(${1:255}, ${2:255}, ${3:255}, ${4:1})', detail: 'Colour with alpha (0-1)' },
];

export function completeCss(
  doc: TextDocument, offset: number, db: UiDb, project: Project, file: string,
  override?: { clean: string; offset: number; base: number; declarationsOnly: boolean },
): CompletionItem[] {
  const clean = override ? override.clean : blankComments(doc.getText());
  const off = override ? override.offset : offset;
  const ctx = cssContextAt(clean, off, override?.declarationsOnly ?? false);
  const base = override ? override.base : 0;
  const range = (start: number, end = off) => ({ start: doc.positionAt(base + start), end: doc.positionAt(base + end) });
  const index = project.indexFor(file);
  const items: CompletionItem[] = [];

  switch (ctx.kind) {
    case 'atRule':
      for (const [label, insert, detail] of [
        ['@import', '@import "$1";', 'Include another style sheet'],
        ['@keyframes', '@keyframes ${1:name} {\n\tfrom { $2 }\n\tto { $3 }\n}', 'Named animation'],
      ]) items.push({ label, kind: CompletionItemKind.Keyword, detail, insertTextFormat: InsertTextFormat.Snippet, textEdit: TextEdit.replace(range(ctx.start), insert) });
      break;

    case 'importPath': {
      const root = gameRootOf(file);
      if (!root) break;
      for (const f of project.listFiles(root, ['.css'])) {
        const rel = f.slice(root.length + 1);
        items.push({ label: rel, kind: CompletionItemKind.File, textEdit: TextEdit.replace(range(ctx.start), rel) });
      }
      break;
    }

    case 'selector':
      if (ctx.sub === 'class') {
        for (const [name, use] of index.classes) items.push({ label: name, kind: CompletionItemKind.Class, detail: path.basename(use.file), textEdit: TextEdit.replace(range(ctx.start), name) });
      } else if (ctx.sub === 'id') {
        for (const [name, use] of index.ids) items.push({ label: name, kind: CompletionItemKind.Reference, detail: path.basename(use.file), textEdit: TextEdit.replace(range(ctx.start), name) });
      } else if (ctx.sub === 'pseudo') {
        for (const p of db.pseudoClasses) items.push({ label: p, kind: CompletionItemKind.Event, detail: 'state', textEdit: TextEdit.replace(range(ctx.start), p) });
      } else if (ctx.sub === 'keyframe') {
        for (const k of ['from', 'to', '0%', '25%', '50%', '75%', '100%']) items.push({ label: k, kind: CompletionItemKind.Value, textEdit: TextEdit.replace(range(ctx.start), k) });
      } else {
        for (const t of db.tags.values()) {
          if (t.legacy) continue;
          items.push({ label: t.name, kind: CompletionItemKind.Class, detail: t.category, documentation: t.description, textEdit: TextEdit.replace(range(ctx.start), t.name) });
        }
        items.push({ label: '*', kind: CompletionItemKind.Operator, detail: 'every element' });
      }
      break;

    case 'property':
      for (const p of db.css.values()) {
        items.push({
          label: p.name, kind: CompletionItemKind.Property, detail: p.grammar, documentation: propDoc(p),
          insertTextFormat: InsertTextFormat.Snippet, textEdit: TextEdit.replace(range(ctx.start), p.name + ': $0;'),
          command: TRIGGER,
        });
      }
      for (const name of new Set([...index.customProps.keys(), ...db.themeTokens])) {
        items.push({ label: name, kind: CompletionItemKind.Variable, detail: 'custom property', textEdit: TextEdit.replace(range(ctx.start), name + ': $0;'), insertTextFormat: InsertTextFormat.Snippet });
      }
      break;

    case 'varName': {
      const names = new Set<string>([...db.themeTokens, ...index.customProps.keys()]);
      for (const name of names) {
        const known = index.customProps.get(name);
        items.push({ label: name, kind: CompletionItemKind.Variable, detail: known?.value, textEdit: TextEdit.replace(range(ctx.start), name) });
      }
      break;
    }

    case 'value': {
      const prop = db.css.get(ctx.property);
      const add = (label: string, kind: CompletionItemKind, detail?: string) =>
        items.push({ label, kind, detail, textEdit: TextEdit.replace(range(ctx.start), label) });
      if (prop) {
        for (const k of prop.keywords) add(k, CompletionItemKind.Value);
        if (prop.types.some((t) => t === 'color')) {
          add('transparent', CompletionItemKind.Color);
        }
        if (prop.types.some((t) => /timing-function/.test(t))) for (const e of db.easings) add(e.replace('()', ''), CompletionItemKind.Value, 'easing');
        if (prop.name === 'animation-name' || prop.name === 'animation') for (const k of index.keyframes.keys()) add(k, CompletionItemKind.Reference, '@keyframes');
        if (prop.name.startsWith('transition')) for (const a of db.animatable) add(a, CompletionItemKind.Property, 'animatable');
      }
      for (const f of NAMED_FUNCTIONS) {
        if (f.label !== 'var()' && !(prop?.types.includes('color'))) continue;
        items.push({ label: f.label, kind: CompletionItemKind.Function, detail: f.detail, insertTextFormat: InsertTextFormat.Snippet, textEdit: TextEdit.replace(range(ctx.start), f.insert) });
      }
      if (ctx.property.startsWith('--')) {
        for (const f of NAMED_FUNCTIONS) if (f.label !== 'var()') items.push({ label: f.label, kind: CompletionItemKind.Function, detail: f.detail, insertTextFormat: InsertTextFormat.Snippet, textEdit: TextEdit.replace(range(ctx.start), f.insert) });
      }
      break;
    }
    default:
      break;
  }
  return items;
}

function propDoc(p: CssProp): { kind: 'markdown'; value: string } {
  const lines = [`\`${p.name}: ${p.grammar}\``, '', `initial: \`${p.initial}\` · ${p.inherited ? 'inherited' : 'not inherited'}${p.shorthand ? ' · shorthand' : ''}`];
  if (p.invalidates) lines.push(`changing it redoes: ${p.invalidates}`);
  return { kind: 'markdown', value: lines.join('\n') };
}

export function hoverCss(doc: TextDocument, offset: number, db: UiDb, project: Project, file: string): Hover | null {
  const text = doc.getText();
  const clean = blankComments(text);
  const word = wordAt(clean, offset, /[\w-]/);
  if (!word) return null;
  const index = project.indexFor(file);
  const mk = (value: string): Hover => ({ contents: { kind: MarkupKind.Markdown, value }, range: { start: doc.positionAt(word.start), end: doc.positionAt(word.end) } });

  if (word.text.startsWith('--')) {
    const known = index.customProps.get(word.text);
    return mk(`**${word.text}**` + (known ? `\n\n\`${known.value}\`\n\n${path.basename(known.file)}` : db.themeTokens.includes(word.text) ? '\n\ntheme token' : '\n\n_not defined anywhere_'));
  }
  const before = clean[word.start - 1];
  if (before === '.') {
    const u = index.classes.get(word.text);
    return mk(`class **.${word.text}**` + (u ? `\n\n${path.basename(u.file)}` : ''));
  }
  if (before === ':') return db.pseudoClasses.includes(word.text) ? mk(`state **:${word.text}**`) : null;
  const p = db.css.get(word.text);
  if (p) return mk(propDoc(p).value);
  const t = db.tags.get(word.text);
  if (t) return mk(`**${t.name}** (${t.category})\n\n${t.description}`);
  return null;
}

export function diagnoseCss(doc: TextDocument, db: UiDb, project: Project, file: string): Diagnostic[] {
  const text = doc.getText();
  const sheet = parseCss(text);
  const out: Diagnostic[] = [];
  const index = project.indexFor(file);
  const root = gameRootOf(file) ?? path.dirname(file);
  const range = (s: number, e: number) => ({ start: doc.positionAt(s), end: doc.positionAt(e) });

  for (const p of sheet.problems) out.push({ range: range(p.start, p.end), message: p.message, severity: DiagnosticSeverity.Error, source: 'engine-ui' });
  for (const im of sheet.imports) {
    if (!resolveGamePath(root, file, im.src)) out.push({ range: range(im.start, im.end), message: `Cannot find style sheet '${im.src}'`, severity: DiagnosticSeverity.Warning, source: 'engine-ui' });
  }
  for (const rule of sheet.rules) {
    if (!rule.inKeyframes) diagnoseSelector(rule.selector, rule.selectorStart, db, out, range);
    for (const d of rule.decls) checkDecl(d, db, index, sheet, out, range);
  }
  return out;
}

function diagnoseSelector(sel: string, base: number, db: UiDb, out: Diagnostic[], range: (s: number, e: number) => { start: any; end: any }): void {
  for (const m of sel.matchAll(/:{1,2}([\w-]+)/g)) {
    if (!db.pseudoClasses.includes(m[1])) {
      out.push({ range: range(base + m.index!, base + m.index! + m[0].length), message: `Unknown state ':${m[1]}'. Known: ${db.pseudoClasses.join(', ')}`, severity: DiagnosticSeverity.Warning, source: 'engine-ui' });
    }
  }
  for (const m of sel.matchAll(/(?:^|[\s>+~,(])([A-Z][\w]*)/g)) {
    const name = m[1];
    if (!db.tags.has(name)) {
      const at = base + m.index! + m[0].length - name.length;
      out.push({ range: range(at, at + name.length), message: `Unknown element type '${name}'`, severity: DiagnosticSeverity.Warning, source: 'engine-ui' });
    }
  }
}

function checkDecl(d: { name: string; nameStart: number; nameEnd: number; value: string; valueStart: number; valueEnd: number }, db: UiDb, index: ReturnType<Project['indexFor']>, _sheet: Sheet, out: Diagnostic[], range: (s: number, e: number) => any): void {
  if (d.value === '' && !d.name.includes(':')) {
    out.push({ range: range(d.nameStart, d.nameEnd), message: `'${d.name}' needs ": value"`, severity: DiagnosticSeverity.Error, source: 'engine-ui' });
    return;
  }
  if (!d.name.startsWith('--') && !db.css.has(d.name)) {
    const near = nearest(d.name, [...db.css.keys()]);
    out.push({ range: range(d.nameStart, d.nameEnd), message: `Unknown property '${d.name}'` + (near ? `. Did you mean '${near}'?` : ''), severity: DiagnosticSeverity.Warning, source: 'engine-ui' });
  }
  if (d.value === '') out.push({ range: range(d.nameStart, d.nameEnd), message: `'${d.name}' has no value`, severity: DiagnosticSeverity.Error, source: 'engine-ui' });
  for (const m of d.value.matchAll(/var\(\s*(--[\w-]+)/g)) {
    if (!index.customProps.has(m[1]) && !db.themeTokens.includes(m[1])) {
      const at = d.valueStart + m.index! + m[0].length - m[1].length;
      out.push({ range: range(at, at + m[1].length), message: `Custom property '${m[1]}' is not defined in any style sheet`, severity: DiagnosticSeverity.Warning, source: 'engine-ui' });
    }
  }
  for (const m of d.value.matchAll(/\(/g)) void m;
  let depth = 0;
  for (const c of d.value) { if (c === '(') depth++; else if (c === ')') depth--; }
  if (depth !== 0) out.push({ range: range(d.valueStart, d.valueEnd), message: 'Unbalanced parentheses', severity: DiagnosticSeverity.Error, source: 'engine-ui' });
}

// Same checks for the style="" attribute of the markup.
export function diagnoseDeclarations(text: string, base: number, db: UiDb, project: Project, file: string, doc: TextDocument): Diagnostic[] {
  const out: Diagnostic[] = [];
  const clean = blankComments(text);
  const index = project.indexFor(file);
  const range = (s: number, e: number) => ({ start: doc.positionAt(base + s), end: doc.positionAt(base + e) });
  for (const d of parseDecls(clean, 0, clean.length)) checkDecl(d, db, index, null as unknown as Sheet, out, range);
  return out;
}

export function cssSymbols(doc: TextDocument): DocumentSymbol[] {
  const sheet = parseCss(doc.getText());
  return sheet.rules.map((r) => ({
    name: r.selector.replace(/\s+/g, ' '), kind: r.inKeyframes ? SymbolKind.Event : SymbolKind.Class,
    range: { start: doc.positionAt(r.selectorStart), end: doc.positionAt(r.bodyEnd + 1) },
    selectionRange: { start: doc.positionAt(r.selectorStart), end: doc.positionAt(r.selectorEnd) },
  }));
}

export function definitionCss(doc: TextDocument, offset: number, project: Project, file: string): Location | null {
  const clean = blankComments(doc.getText());
  const word = wordAt(clean, offset, /[\w-]/);
  if (!word) return null;
  const index = project.indexFor(file);
  const before = clean[word.start - 1];
  const root = gameRootOf(file) ?? path.dirname(file);
  const sheet = parseCss(doc.getText());
  for (const im of sheet.imports) {
    if (offset >= im.start && offset <= im.end) {
      const target = resolveGamePath(root, file, im.src);
      return target ? { uri: URI.file(target).toString(), range: { start: { line: 0, character: 0 }, end: { line: 0, character: 0 } } } : null;
    }
  }
  let use;
  if (word.text.startsWith('--')) use = index.customProps.get(word.text);
  else if (before === '.') use = index.classes.get(word.text);
  else if (before === '#') use = index.ids.get(word.text);
  else use = index.keyframes.get(word.text);
  if (!use) return null;
  return locationOf(use.file, use.offset, word.text.length, project);
}

export function locationOf(file: string, offset: number, length: number, project: Project): Location {
  const text = project.textOf(file) ?? require('fs').readFileSync(file, 'utf8');
  const d = TextDocument.create(URI.file(file).toString(), 'uicss', 0, text);
  return { uri: d.uri, range: { start: d.positionAt(offset), end: d.positionAt(offset + length) } };
}

export function colorsCss(doc: TextDocument): ColorInformation[] {
  const text = blankComments(doc.getText());
  return findColors(text).map((c) => ({ range: { start: doc.positionAt(c.start), end: doc.positionAt(c.end) }, color: Color.create(c.r, c.g, c.b, c.a) }));
}

export function colorPresentations(color: Color): ColorPresentation[] {
  const to = (v: number) => Math.round(v * 255);
  const hex = (v: number) => to(v).toString(16).padStart(2, '0');
  const a = Math.round(color.alpha * 100) / 100;
  return [
    { label: color.alpha < 1 ? `rgba(${to(color.red)}, ${to(color.green)}, ${to(color.blue)}, ${a})` : `#${hex(color.red)}${hex(color.green)}${hex(color.blue)}` },
    { label: `rgba(${to(color.red)}, ${to(color.green)}, ${to(color.blue)}, ${a})` },
    { label: `#${hex(color.red)}${hex(color.green)}${hex(color.blue)}${color.alpha < 1 ? hex(color.alpha) : ''}` },
  ];
}

export function wordAt(text: string, offset: number, ch: RegExp): { text: string; start: number; end: number } | null {
  let s = offset, e = offset;
  while (s > 0 && ch.test(text[s - 1])) s--;
  while (e < text.length && ch.test(text[e])) e++;
  return e > s ? { text: text.slice(s, e), start: s, end: e } : null;
}

export function nearest(name: string, candidates: string[]): string | undefined {
  let best: string | undefined, bestD = 3;
  for (const c of candidates) {
    const d = distance(name, c);
    if (d < bestD) { bestD = d; best = c; }
  }
  return best;
}

function distance(a: string, b: string): number {
  if (Math.abs(a.length - b.length) > 2) return 99;
  const row = Array.from({ length: b.length + 1 }, (_, i) => i);
  for (let i = 1; i <= a.length; i++) {
    let prev = row[0];
    row[0] = i;
    for (let j = 1; j <= b.length; j++) {
      const tmp = row[j];
      row[j] = Math.min(row[j] + 1, row[j - 1] + 1, prev + (a[i - 1] === b[j - 1] ? 0 : 1));
      prev = tmp;
    }
  }
  return row[b.length];
}
