import * as fs from 'fs';

// The engine's own description of its UI language (SaveData/UIStudio/uidb.json, written by the game).
// Everything the server knows about tags, attributes, CSS properties and actions comes from here.

export interface AttrDef {
  name: string;
  type: string;
  description: string;
  default: string;
  required: boolean;
  pattern?: RegExp;          // set for entries such as param-<name>
}

export interface TagDef {
  name: string;
  category: string;
  description: string;
  legacy: boolean;
  component: boolean;
  path?: string;             // component file
  attrs: AttrDef[];
  byName: Map<string, AttrDef>;
}

export interface CssProp {
  name: string;
  grammar: string;
  inherited: boolean;
  initial: string;
  invalidates: string;
  shorthand: boolean;
  keywords: string[];        // bare words of the grammar, offered as value completions
  types: string[];           // <color>, <length> ... used to pick extra suggestions
}

export interface ActionDef {
  name: string;
  arguments: string;
  description: string;
}

export class UiDb {
  tags = new Map<string, TagDef>();
  css = new Map<string, CssProp>();
  actions = new Map<string, ActionDef>();
  pseudoClasses: string[] = [];
  themeTokens: string[] = [];
  easings: string[] = [];
  animatable: string[] = [];
  localeKeys = new Set<string>();
  inputActions: string[] = [];
  commonAttrs: AttrDef[] = [];
  source = '';

  static load(file: string): UiDb {
    const db = new UiDb();
    db.source = file;
    const root = JSON.parse(fs.readFileSync(file, 'utf8'));
    const reg = root.registries ?? {};

    for (const t of reg.tags ?? []) {
      const def: TagDef = {
        name: t.name, category: t.category ?? '', description: t.description ?? '',
        legacy: !!t.legacy || t.category === 'legacy', component: false, attrs: [], byName: new Map(),
      };
      for (const a of t.attributes ?? []) {
        const attr: AttrDef = {
          name: a.name, type: a.type ?? 'string', description: a.description ?? '',
          default: a.default === undefined || a.default === null ? '' : String(a.default), required: !!a.required,
        };
        if (/[<>]/.test(a.name)) {
          const escaped = a.name.replace(/[.*+?^${}()|[\]\\]/g, '\\$&').replace(/<[^>]*>/g, '[\\w.-]+');
          attr.pattern = new RegExp('^' + escaped + '$');
        }
        def.attrs.push(attr);
        if (!attr.pattern) def.byName.set(attr.name, attr);
      }
      db.tags.set(def.name, def);
    }

    // attributes every built-in tag has: components get these too
    const builtin = [...db.tags.values()].filter((t) => !t.legacy);
    if (builtin.length > 0) {
      db.commonAttrs = builtin[0].attrs.filter((a) => !a.pattern && builtin.every((t) => t.byName.has(a.name)));
    }

    for (const c of reg.components ?? []) {
      const def: TagDef = {
        name: c.tag, category: 'component', description: 'Reusable component (' + c.path + ')',
        legacy: false, component: true, path: c.path, attrs: [...db.commonAttrs], byName: new Map(),
      };
      for (const p of c.parameters ?? []) {
        def.attrs.push({
          name: p.name, type: 'string', description: 'Component parameter' + (p.required ? ' (required)' : ''),
          default: p.default === undefined ? '' : String(p.default), required: !!p.required,
        });
      }
      for (const a of def.attrs) def.byName.set(a.name, a);
      db.tags.set(def.name, def);
    }

    for (const p of reg.cssProperties ?? []) {
      const grammar: string = p.grammar ?? '';
      db.css.set(p.name, {
        name: p.name, grammar, inherited: !!p.inherited, initial: String(p.initial ?? ''),
        invalidates: p.invalidates ?? '', shorthand: !!p.shorthand,
        keywords: grammarKeywords(grammar), types: (grammar.match(/<[^>]*>/g) ?? []).map((s) => s.slice(1, -1)),
      });
    }

    for (const a of reg.actions ?? []) {
      if (a.kind === 'action') db.actions.set(a.name, { name: a.name, arguments: a.arguments ?? '', description: a.description ?? '' });
    }
    db.pseudoClasses = (reg.pseudoClasses ?? []).map((p: { name: string }) => p.name);
    db.themeTokens = (reg.themeTokens ?? []).filter((t: { kind: string }) => t.kind === 'custom-property').map((t: { name: string }) => t.name);
    db.easings = (reg.easings ?? []).map((e: { name: string }) => e.name);
    db.animatable = (reg.animations ?? []).filter((a: { kind: string }) => a.kind === 'animatable-property').map((a: { name: string }) => a.name);
    for (const l of reg.localeKeys ?? []) for (const k of l.keys ?? []) db.localeKeys.add(k);
    db.inputActions = (reg.inputActions ?? []).map((i: { name: string }) => i.name);
    return db;
  }
}

// "none | solid | [<name> || <time>]#" -> ["none", "solid"]; words inside <...> are types, not values.
export function grammarKeywords(grammar: string): string[] {
  let g = grammar.replace(/,\s*\d.*$/, '');        // "..., 1-4 values" is a remark, not grammar
  g = g.replace(/<[^>]*>/g, ' ');
  const out = new Set<string>();
  for (const m of g.matchAll(/(?<![\w-])[a-z][a-z0-9-]*(?![\w(-])/g)) out.add(m[0]);
  return [...out];
}
