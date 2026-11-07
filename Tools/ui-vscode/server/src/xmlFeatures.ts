import * as path from 'path';
import {
  CompletionItem, CompletionItemKind, Color, ColorInformation, Diagnostic, DiagnosticSeverity, DocumentSymbol,
  Hover, InsertTextFormat, Location, MarkupKind, SymbolKind, TextEdit,
} from 'vscode-languageserver';
import { TextDocument } from 'vscode-languageserver-textdocument';
import { URI } from 'vscode-uri';
import { blankComments, findColors } from './css';
import { completeCss, diagnoseDeclarations, locationOf, nearest, wordAt } from './cssFeatures';
import { AttrDef, TagDef, UiDb } from './db';
import { gameRootOf, Project, resolveGamePath } from './project';
import { Attr, contextAt, El, enclosingElement, scanXml } from './xml';

const TRIGGER = { command: 'editor.action.triggerSuggest', title: 'Suggest' };

// Elements that structure a document and are not widgets.
const STRUCTURAL: Record<string, { description: string; attrs: AttrDef[]; children?: string[] }> = {
  root: { description: 'Document element. Children in order: imports, styles, snippets, then exactly one top element.', attrs: [], children: ['imports', 'styles', 'snippets'] },
  imports: { description: 'Component files used by this document.', attrs: [], children: ['import'] },
  import: { description: 'Brings the components of a file into this document.', attrs: [attr('src', 'path', 'Path to the component file, e.g. GameData/ui/kit/components/Card.xml', true)] },
  styles: { description: 'Style sheets of this document, applied in order.', attrs: [], children: ['include', 'style'] },
  include: { description: 'Loads a .css file.', attrs: [attr('src', 'path', 'Path to the style sheet, e.g. GameData/ui/theme.css', true)] },
  style: { description: 'Inline style sheet. Write rules as in a .css file.', attrs: [] },
  snippets: { description: 'Small reusable pieces local to this document.', attrs: [], children: ['snippet'] },
  snippet: { description: 'A reusable piece. Use it as a tag named like the snippet; params are filled with {name}.', attrs: [attr('name', 'string', 'Tag name to use it with', true), attr('params', 'string', "Comma separated: name or name='default'", false)] },
  Children: { description: 'Inside a component: where the content written between the component tags goes.', attrs: [] },
  Component: { description: 'Top element of a component file: defines a new tag.', attrs: [attr('tag', 'string', 'Name of the new tag', true), attr('params', 'string', "Comma separated parameters: name (required) or name='default'", false)] },
};

function attr(name: string, type: string, description: string, required: boolean): AttrDef {
  return { name, type, description, default: '', required };
}

interface Resolved {
  def: TagDef | null;
  structural: boolean;
  extra: AttrDef[];           // parameters of snippets / imported components
  looseAttrs: boolean;        // any attribute is accepted (snippet invocations, param-*)
}

function resolveTag(name: string, db: UiDb, local: ReturnType<Project['documentTags']>): Resolved | null {
  const s = STRUCTURAL[name];
  if (s) return { def: { name, category: 'structure', description: s.description, legacy: false, component: false, attrs: s.attrs, byName: new Map(s.attrs.map((a) => [a.name, a])) }, structural: true, extra: [], looseAttrs: false };
  const snip = local.get(name);
  if (snip) {
    const extra = snip.params.map((p) => attr(p, 'string', snip.kind === 'snippet' ? 'Snippet parameter' : 'Component parameter', false));
    const base = db.tags.get('Panel');
    return { def: db.tags.get(name) ?? { name, category: snip.kind, description: snip.kind === 'snippet' ? 'Snippet of this document' : 'Imported component', legacy: false, component: true, path: snip.path, attrs: [], byName: new Map() }, structural: false, extra: [...(base ? db.commonAttrs : []), ...extra], looseAttrs: false };
  }
  const def = db.tags.get(name);
  return def ? { def, structural: false, extra: [], looseAttrs: false } : null;
}

function allAttrs(r: Resolved): AttrDef[] {
  return [...(r.def?.attrs ?? []), ...r.extra].filter((a, i, arr) => arr.findIndex((b) => b.name === a.name) === i);
}

function findAttr(r: Resolved, name: string): AttrDef | undefined {
  return allAttrs(r).find((a) => (a.pattern ? a.pattern.test(name) : a.name === name));
}

export function completeXml(doc: TextDocument, offset: number, db: UiDb, project: Project, file: string): CompletionItem[] {
  const text = doc.getText();
  const scan = scanXml(text);
  const ctx = contextAt(scan, text, offset);
  const local = project.documentTags(file, text);
  const items: CompletionItem[] = [];
  const range = (s: number, e = offset) => ({ start: doc.positionAt(s), end: doc.positionAt(e) });

  // ${KEY} anywhere inside a value or text
  const lineStart = text.lastIndexOf('\n', offset - 1) + 1;
  const interp = /\$\{([\w.]*)$/.exec(text.slice(lineStart, offset));
  if (interp && ctx.kind !== 'tagName' && ctx.kind !== 'attrName') {
    const start = offset - interp[1].length;
    for (const key of db.localeKeys) items.push({ label: key, kind: CompletionItemKind.Text, detail: 'localisation key', textEdit: TextEdit.replace(range(start), key) });
    return items;
  }

  switch (ctx.kind) {
    case 'closeTag': {
      const open = enclosingElement(scan, ctx.start - 2);
      if (open) items.push({ label: open.name, kind: CompletionItemKind.Class, detail: 'close the open element', textEdit: TextEdit.replace(range(ctx.start), open.name + '>') });
      break;
    }

    case 'tagName': {
      const parent = ctx.el.parent;
      const parentRule = parent ? STRUCTURAL[parent.name] : undefined;
      if (!parent) {
        items.push(tagItem('root', STRUCTURAL.root.description, 'structure', range(ctx.start), true));
        break;
      }
      if (parentRule?.children) {
        for (const c of parentRule.children) items.push(tagItem(c, STRUCTURAL[c].description, 'structure', range(ctx.start), c !== 'include' && c !== 'import'));
        if (parent.name !== 'root') break;                  // below <root> the top element may follow too
      }
      if (parent.name === 'style') break;
      for (const t of db.tags.values()) {
        if (t.legacy) continue;
        items.push(tagItem(t.name, t.description, t.category, range(ctx.start), !t.attrs.some((a) => a.name === 'text') || t.category === 'layout', t));
      }
      for (const [name, snip] of local) {
        items.push(tagItem(name, snip.kind === 'snippet' ? 'Snippet of this document' : 'Imported component', snip.kind, range(ctx.start), false));
      }
      if (parent.name === 'root' && !scan.all.some((e) => e.name === 'Component')) items.push(tagItem('Component', STRUCTURAL.Component.description, 'structure', range(ctx.start), true));
      break;
    }

    case 'attrName': {
      const r = resolveTag(ctx.el.name, db, local);
      if (!r) break;
      const present = new Set(ctx.el.attrs.filter((a) => a.nameStart !== ctx.start).map((a) => a.name));
      for (const a of allAttrs(r)) {
        if (a.pattern || present.has(a.name)) continue;
        const required = a.required ? ' (required)' : '';
        let insert = ctx.hasValue ? a.name : `${a.name}="$1"`;
        if (!ctx.hasValue && a.type === 'bool') insert = `${a.name}="\${1|true,false|}"`;
        items.push({
          label: a.name, kind: a.name.startsWith('on') ? CompletionItemKind.Event : CompletionItemKind.Property,
          detail: `${a.type}${a.default ? ' = ' + a.default : ''}${required}`, documentation: a.description,
          sortText: (a.required ? '0' : isCommon(a, db) ? '2' : '1') + a.name, insertTextFormat: InsertTextFormat.Snippet,
          textEdit: TextEdit.replace(range(ctx.start, ctx.end), insert), command: ctx.hasValue || a.type === 'bool' ? undefined : TRIGGER,
        });
      }
      break;
    }

    case 'attrValue': {
      const r = resolveTag(ctx.el.name, db, local);
      const def = r ? findAttr(r, ctx.attr.name) : undefined;
      items.push(...completeValue(doc, offset, ctx.el, ctx.attr, def, db, project, file, text));
      break;
    }
    default:
      break;
  }
  return items;
}

function isCommon(a: AttrDef, db: UiDb): boolean {
  return db.commonAttrs.some((c) => c.name === a.name);
}

function tagItem(name: string, description: string, detail: string, range: { start: any; end: any }, container: boolean, def?: TagDef): CompletionItem {
  const required = def?.attrs.filter((a) => a.required && !a.pattern).map((a, i) => `${a.name}="$${i + 1}"`).join(' ') ?? '';
  const open = name + (required ? ' ' + required : '');
  return {
    label: name, kind: CompletionItemKind.Class, detail, documentation: description,
    insertTextFormat: InsertTextFormat.Snippet,
    textEdit: TextEdit.replace(range as any, container ? `${open}>\n\t$0\n</${name}>` : `${open}$0/>`),
    filterText: name, sortText: def?.category === 'legacy' ? 'z' + name : name,
  };
}

function completeValue(doc: TextDocument, offset: number, el: El, a: Attr, def: AttrDef | undefined, db: UiDb, project: Project, file: string, text: string): CompletionItem[] {
  const items: CompletionItem[] = [];
  const range = (s: number, e = offset) => ({ start: doc.positionAt(s), end: doc.positionAt(e) });
  const typed = text.slice(a.valueStart, offset);

  if (a.name === 'style') {
    const value = text.slice(a.valueStart, a.valueEnd);
    return completeCss(doc, offset, db, project, file, { clean: blankComments(value), offset: offset - a.valueStart, base: a.valueStart, declarationsOnly: true });
  }
  const root = gameRootOf(file);

  if (a.name === 'class') {
    const word = /[\w-]*$/.exec(typed)![0];
    const used = new Set(a.value.split(/\s+/));
    for (const [name, use] of project.indexFor(file).classes) {
      if (used.has(name) && name !== word) continue;
      items.push({ label: name, kind: CompletionItemKind.Class, detail: path.basename(use.file), textEdit: TextEdit.replace(range(offset - word.length), name) });
    }
    return items;
  }

  if (a.name === 'src' && (el.name === 'include' || el.name === 'import')) {
    if (root) {
      for (const f of project.listFiles(root, [el.name === 'include' ? '.css' : '.xml'])) {
        const rel = f.slice(root.length + 1);
        items.push({ label: rel, kind: CompletionItemKind.File, textEdit: TextEdit.replace(range(a.valueStart, a.valueEnd), rel) });
      }
    }
    return items;
  }

  if (def?.type === 'actions' || /^on[a-z]+$/.test(a.name)) {
    const word = /[A-Za-z_][\w]*$/.exec(typed)?.[0] ?? '';
    for (const act of db.actions.values()) {
      items.push({
        label: act.name, kind: CompletionItemKind.Function, detail: act.arguments ? `(${act.arguments})` : '()', documentation: act.description,
        insertTextFormat: InsertTextFormat.Snippet, textEdit: TextEdit.replace(range(offset - word.length), act.name + (act.arguments ? '($1)' : '()')),
      });
    }
    return items;
  }

  switch (def?.type) {
    case 'bool':
      for (const v of ['true', 'false']) items.push({ label: v, kind: CompletionItemKind.Value, textEdit: TextEdit.replace(range(a.valueStart, a.valueEnd), v) });
      break;
    case 'color':
      for (const f of ['#ffffff', '#000000', 'rgba(255, 255, 255, 1)']) items.push({ label: f, kind: CompletionItemKind.Color, textEdit: TextEdit.replace(range(a.valueStart, a.valueEnd), f) });
      break;
    case 'localisation-key':
      for (const k of db.localeKeys) items.push({ label: '${' + k + '}', kind: CompletionItemKind.Text, filterText: k, textEdit: TextEdit.replace(range(a.valueStart, a.valueEnd), '${' + k + '}') });
      break;
    case 'path':
    case 'asset':
      if (root) {
        const exts = /image|icon|texture|asset/i.test(a.name) || def.type === 'asset' ? ['.png', '.jpg', '.jpeg', '.tga', '.bmp', '.webp', '.svg', '.ktx', '.dds'] : ['.xml', '.css'];
        for (const f of project.listFiles(root, exts)) {
          const rel = f.slice(root.length + 1);
          items.push({ label: rel, kind: CompletionItemKind.File, textEdit: TextEdit.replace(range(a.valueStart, a.valueEnd), rel) });
        }
      }
      break;
    default:
      if (a.name === 'text' || a.name === 'label') {
        items.push({ label: '${…}', kind: CompletionItemKind.Text, detail: 'localised text', insertTextFormat: InsertTextFormat.Snippet, textEdit: TextEdit.replace(range(offset), '${$1}'), command: TRIGGER });
      }
      break;
  }
  return items;
}

export function hoverXml(doc: TextDocument, offset: number, db: UiDb, project: Project, file: string): Hover | null {
  const text = doc.getText();
  const scan = scanXml(text);
  const ctx = contextAt(scan, text, offset);
  const local = project.documentTags(file, text);
  const md = (value: string, s: number, e: number): Hover => ({ contents: { kind: MarkupKind.Markdown, value }, range: { start: doc.positionAt(s), end: doc.positionAt(e) } });

  const el = scan.all.find((e) => offset >= e.nameStart && offset <= e.nameEnd) ??
    scan.all.find((e) => e.closeStart >= 0 && offset > e.closeStart && offset < e.closeEnd);
  if (el) {
    const r = resolveTag(el.name, db, local);
    if (!r?.def) return null;
    const attrs = allAttrs(r).filter((a) => !a.pattern && !isCommon(a, db)).map((a) => '`' + a.name + '`').join(' ');
    return md(`**${el.name}** · ${r.def.category}${r.def.legacy ? ' · legacy' : ''}\n\n${r.def.description}${attrs ? '\n\nAttributes: ' + attrs : ''}`, el.nameStart, el.nameEnd);
  }
  if (ctx.kind === 'attrName' && ctx.prefix !== '') {
    const at = ctx.el.attrs.find((a) => a.nameStart === ctx.start);
    const r = resolveTag(ctx.el.name, db, local);
    const def = at && r ? findAttr(r, at.name) : undefined;
    if (at && def) {
      return md(`**${at.name}** · ${def.type}${def.default ? ` · default \`${def.default}\`` : ''}${def.required ? ' · required' : ''}\n\n${def.description}`, at.nameStart, at.nameEnd);
    }
  }
  if (ctx.kind === 'attrValue') {
    const key = /\$\{([\w.]*)\}/g;
    for (const m of ctx.attr.value.matchAll(key)) {
      const s = ctx.attr.valueStart + m.index!;
      if (offset >= s && offset <= s + m[0].length) return md(db.localeKeys.has(m[1]) ? `localisation key \`${m[1]}\`` : `localisation key \`${m[1]}\` — not in the loaded language`, s, s + m[0].length);
    }
    if (ctx.attr.name === 'class') {
      const w = wordAt(text, offset, /[\w-]/);
      const use = w && project.indexFor(file).classes.get(w.text);
      if (w && use) return md(`class **.${w.text}** — ${path.basename(use.file)}`, w.start, w.end);
    }
    if (/^on[a-z]+$/.test(ctx.attr.name)) {
      const w = wordAt(text, offset, /\w/);
      const act = w && db.actions.get(w.text);
      if (w && act) return md(`**${act.name}**(${act.arguments})\n\n${act.description}`, w.start, w.end);
    }
  }
  return null;
}

export function diagnoseXml(doc: TextDocument, db: UiDb, project: Project, file: string): Diagnostic[] {
  const text = doc.getText();
  const scan = scanXml(text);
  const local = project.documentTags(file, text);
  const out: Diagnostic[] = [];
  const range = (s: number, e: number) => ({ start: doc.positionAt(s), end: doc.positionAt(e) });
  const push = (s: number, e: number, message: string, severity: DiagnosticSeverity, code?: string) =>
    out.push({ range: range(s, e), message, severity, source: 'engine-ui', code });
  const root = gameRootOf(file) ?? path.dirname(file);
  const index = project.indexFor(file);

  for (const p of scan.problems) push(p.start, p.end, p.message, DiagnosticSeverity.Error);

  if (scan.roots.length > 0 && scan.roots[0].name !== 'root') push(scan.roots[0].nameStart, scan.roots[0].nameEnd, 'The document element must be <root>', DiagnosticSeverity.Error);
  if (scan.roots.length > 1) for (const r of scan.roots.slice(1)) push(r.nameStart, r.nameEnd, 'Only one document element is allowed', DiagnosticSeverity.Error);

  for (const el of scan.all) {
    if (el.name === '') continue;
    const r = resolveTag(el.name, db, local);
    const insideStyle = el.parent?.name === 'style';
    if (insideStyle) continue;
    if (!r) {
      const near = nearest(el.name, [...db.tags.keys(), ...Object.keys(STRUCTURAL), ...local.keys()]);
      push(el.nameStart, el.nameEnd, `Unknown tag '${el.name}'` + (near ? `. Did you mean '${near}'?` : ''), DiagnosticSeverity.Error, 'unknown-tag');
      continue;
    }
    if (r.def?.legacy) push(el.nameStart, el.nameEnd, `'${el.name}' is a legacy tag; use the new widgets`, DiagnosticSeverity.Information, 'legacy');
    const seen = new Set<string>();
    const looseParams = r.def?.component === true && r.def.category !== 'component' ? true : false;
    for (const a of el.attrs) {
      if (seen.has(a.name)) push(a.nameStart, a.nameEnd, `Attribute '${a.name}' is given twice`, DiagnosticSeverity.Warning);
      seen.add(a.name);
      const def = findAttr(r, a.name);
      if (!def) {
        if (looseParams || a.name.startsWith('xmlns')) continue;
        const near = nearest(a.name, allAttrs(r).filter((x) => !x.pattern).map((x) => x.name));
        push(a.nameStart, a.nameEnd, `'${el.name}' has no attribute '${a.name}'` + (near ? `. Did you mean '${near}'?` : ''), DiagnosticSeverity.Warning, 'unknown-attribute');
        continue;
      }
      if (!a.hasValue) { push(a.nameStart, a.nameEnd, `Attribute '${a.name}' needs a value`, DiagnosticSeverity.Error); continue; }
      checkValue(a, def, el, db, index, root, file, doc, out, text, project);
    }
    for (const req of allAttrs(r)) {
      if (req.required && !req.pattern && !seen.has(req.name)) push(el.nameStart, el.nameEnd, `'${el.name}' needs the attribute '${req.name}'`, DiagnosticSeverity.Error, 'missing-attribute');
    }
  }
  return out;
}

function checkValue(a: Attr, def: AttrDef, el: El, db: UiDb, index: ReturnType<Project['indexFor']>, root: string, file: string, doc: TextDocument, out: Diagnostic[], text: string, project: Project): void {
  const range = (s: number, e: number) => ({ start: doc.positionAt(s), end: doc.positionAt(e) });
  const add = (s: number, e: number, message: string, severity: DiagnosticSeverity) => out.push({ range: range(s, e), message, severity, source: 'engine-ui' });
  const v = a.value;
  const dynamic = /\$\{|\{[\w.]+\}/.test(v);

  for (const m of v.matchAll(/\$\{([\w.]*)\}/g)) {
    if (db.localeKeys.size > 0 && !db.localeKeys.has(m[1]) && m[1] !== '') {
      add(a.valueStart + m.index!, a.valueStart + m.index! + m[0].length, `'${m[1]}' is not a localisation key of the loaded language`, DiagnosticSeverity.Information);
    }
  }

  if (a.name === 'style') {
    out.push(...diagnoseDeclarations(v, a.valueStart, db, project, file, doc));
    return;
  }
  if (a.name === 'class') {
    for (const m of v.matchAll(/[^\s]+/g)) {
      if (m[0].includes('${') || m[0].includes('{')) continue;
      if (index.classes.size > 0 && !index.classes.has(m[0])) add(a.valueStart + m.index!, a.valueStart + m.index! + m[0].length, `No style sheet defines class '${m[0]}'`, DiagnosticSeverity.Hint);
    }
    return;
  }
  if (a.name === 'src' && (el.name === 'include' || el.name === 'import')) {
    if (!resolveGamePath(root, file, v)) add(a.valueStart, a.valueEnd, `Cannot find '${v}'`, DiagnosticSeverity.Warning);
    return;
  }
  if (def.type === 'actions' || /^on[a-z]+$/.test(a.name)) {
    for (const m of v.matchAll(/\b([A-Za-z_]\w*)\s*\(/g)) {
      if (!db.actions.has(m[1])) {
        add(a.valueStart + m.index!, a.valueStart + m.index! + m[1].length, `'${m[1]}' is not a built-in action. It runs only if the page's code registers a handler with this name.`, DiagnosticSeverity.Hint);
      }
    }
    return;
  }
  if (dynamic) return;
  switch (def.type) {
    case 'bool':
      if (v !== 'true' && v !== 'false') add(a.valueStart, a.valueEnd, `'${a.name}' must be true or false`, DiagnosticSeverity.Error);
      break;
    case 'float':
    case 'number':
      if (!/^-?(\d+\.?\d*|\.\d+)(px|%|s|ms)?$/.test(v.trim())) add(a.valueStart, a.valueEnd, `'${a.name}' must be a number`, DiagnosticSeverity.Warning);
      break;
    case 'vec2':
      if (!/^\s*-?[\d.]+(px|%)?\s*[, ]\s*-?[\d.]+(px|%)?\s*$/.test(v)) add(a.valueStart, a.valueEnd, `'${a.name}' needs two numbers, like "10, 20"`, DiagnosticSeverity.Warning);
      break;
    case 'color':
      if (!/^(#[0-9a-fA-F]{3,8}|rgba?\([^)]*\)|var\([^)]*\)|transparent)$/.test(v.trim())) add(a.valueStart, a.valueEnd, `'${a.name}' must be a colour: #rrggbb, rgb() or rgba()`, DiagnosticSeverity.Warning);
      break;
    default:
      break;
  }
}

export function symbolsXml(doc: TextDocument): DocumentSymbol[] {
  const scan = scanXml(doc.getText());
  const conv = (el: El): DocumentSymbol => {
    const id = el.attrs.find((a) => a.name === 'id')?.value;
    const cls = el.attrs.find((a) => a.name === 'class')?.value;
    const end = el.closeEnd >= 0 ? el.closeEnd : el.tagEnd + 1;
    return {
      name: el.name + (id ? '#' + id : ''), detail: cls, kind: el.name === 'Component' || el.name === 'snippet' ? SymbolKind.Function : SymbolKind.Object,
      range: { start: doc.positionAt(el.start), end: doc.positionAt(Math.max(end, el.nameEnd)) },
      selectionRange: { start: doc.positionAt(el.nameStart), end: doc.positionAt(Math.max(el.nameEnd, el.nameStart)) },
      children: el.children.map(conv),
    };
  };
  return scan.roots.map(conv);
}

export function definitionXml(doc: TextDocument, offset: number, db: UiDb, project: Project, file: string): Location | null {
  const text = doc.getText();
  const scan = scanXml(text);
  const ctx = contextAt(scan, text, offset);
  const root = gameRootOf(file) ?? path.dirname(file);
  const start = { line: 0, character: 0 };

  const toFile = (target: string | undefined): Location | null => (target ? { uri: URI.file(target).toString(), range: { start, end: start } } : null);

  if (ctx.kind === 'attrValue') {
    const a = ctx.attr;
    if (a.name === 'src' || a.name === 'path') return toFile(resolveGamePath(root, file, a.value));
    if (a.name === 'class') {
      const w = wordAt(text, offset, /[\w-]/);
      const use = w && project.indexFor(file).classes.get(w.text);
      if (w && use) return locationOf(use.file, use.offset, w.text.length, project);
    }
    return null;
  }
  const el = scan.all.find((e) => offset >= e.nameStart && offset <= e.nameEnd);
  if (el) {
    const local = project.documentTags(file, text);
    const loc = local.get(el.name);
    if (loc?.kind === 'snippet') {
      const decl = scan.all.find((e) => e.name === 'snippet' && e.attrs.some((a) => a.name === 'name' && a.value === el.name));
      if (decl) return { uri: doc.uri, range: { start: doc.positionAt(decl.nameStart), end: doc.positionAt(decl.nameEnd) } };
    }
    const comp = db.tags.get(el.name);
    const target = loc?.path ?? (comp?.component && comp.path ? resolveGamePath(root, file, comp.path) : undefined);
    return toFile(target);
  }
  return null;
}

export function colorsXml(doc: TextDocument, db: UiDb): ColorInformation[] {
  const text = doc.getText();
  const scan = scanXml(text);
  const out: ColorInformation[] = [];
  for (const el of scan.all) {
    for (const a of el.attrs) {
      if (!a.hasValue) continue;
      const t = db.tags.get(el.name)?.byName.get(a.name);
      if (t?.type !== 'color' && a.name !== 'style') continue;
      for (const c of findColors(text, a.valueStart, a.valueEnd)) {
        out.push({ range: { start: doc.positionAt(c.start), end: doc.positionAt(c.end) }, color: Color.create(c.r, c.g, c.b, c.a) });
      }
    }
  }
  return out;
}
