import * as fs from 'fs';
import * as path from 'path';
import { parseCss, Sheet } from './css';
import { UiDb } from './db';
import { scanXml } from './xml';

// A "game" is the folder that has GameData/ in it. Every game has its own database, style sheets and components.

export function gameRootOf(file: string): string | null {
  const norm = file.replace(/\\/g, '/');
  const idx = norm.search(/\/GameData\//i);
  if (idx >= 0) return norm.slice(0, idx);
  let dir = path.dirname(file);
  for (let i = 0; i < 8; i++) {
    if (fs.existsSync(path.join(dir, 'GameData'))) return dir.replace(/\\/g, '/');
    const up = path.dirname(dir);
    if (up === dir) break;
    dir = up;
  }
  return null;
}

export type TextProvider = (file: string) => string | undefined;     // contents of an open (maybe unsaved) document

export interface ClassUse { file: string; offset: number }

export class GameIndex {
  classes = new Map<string, ClassUse>();
  ids = new Map<string, ClassUse>();
  customProps = new Map<string, ClassUse & { value: string }>();
  keyframes = new Map<string, ClassUse>();
  sheets = new Map<string, Sheet>();
  snippetsOrImports = 0;
  built = 0;
}

export class Project {
  bundledDatabase = '';
  databaseOverride = '';
  textOf: TextProvider = () => undefined;

  private dbCache = new Map<string, { mtime: number; db: UiDb }>();
  private indexCache = new Map<string, GameIndex>();
  private fileCache = new Map<string, { at: number; files: string[] }>();

  dbFor(file: string): UiDb {
    const candidates: string[] = [];
    if (this.databaseOverride) candidates.push(this.databaseOverride);
    const root = gameRootOf(file);
    if (root) candidates.push(path.join(root, 'SaveData', 'UIStudio', 'uidb.json'));
    if (this.bundledDatabase) candidates.push(this.bundledDatabase);
    for (const c of candidates) {
      try {
        const st = fs.statSync(c);
        const hit = this.dbCache.get(c);
        if (hit && hit.mtime === st.mtimeMs) return hit.db;
        const db = UiDb.load(c);
        this.dbCache.set(c, { mtime: st.mtimeMs, db });
        return db;
      } catch { /* try the next one */ }
    }
    throw new Error('No uidb.json found. Run the game once (it writes SaveData/UIStudio/uidb.json) or set engineUi.databasePath.');
  }

  invalidate(): void {
    this.indexCache.clear();
    this.fileCache.clear();
  }

  // classes, ids, custom properties and keyframes of every style sheet below GameData/
  indexFor(file: string): GameIndex {
    const root = gameRootOf(file) ?? path.dirname(file);
    let idx = this.indexCache.get(root);
    if (idx) return idx;
    idx = new GameIndex();
    for (const css of this.listFiles(root, ['.css'])) {
      const text = this.textOf(css) ?? safeRead(css);
      if (text === undefined) continue;
      const sheet = parseCss(text);
      idx.sheets.set(css, sheet);
      for (const [k, off] of sheet.classes) if (!idx.classes.has(k)) idx.classes.set(k, { file: css, offset: off });
      for (const [k, off] of sheet.ids) if (!idx.ids.has(k)) idx.ids.set(k, { file: css, offset: off });
      for (const [k, v] of sheet.customProps) if (!idx.customProps.has(k)) idx.customProps.set(k, { file: css, offset: v.offset, value: v.value });
      for (const k of sheet.keyframes) if (!idx.keyframes.has(k.name)) idx.keyframes.set(k.name, { file: css, offset: k.start });
    }
    this.indexCache.set(root, idx);
    return idx;
  }

  // all files under <root>/GameData with one of the extensions (cached for a few seconds)
  listFiles(root: string, exts: string[]): string[] {
    const key = root + '|' + exts.join(',');
    const hit = this.fileCache.get(key);
    if (hit && Date.now() - hit.at < 5000) return hit.files;
    const files: string[] = [];
    const walk = (dir: string, depth: number) => {
      if (depth > 8 || files.length > 20000) return;
      let entries: fs.Dirent[];
      try { entries = fs.readdirSync(dir, { withFileTypes: true }); } catch { return; }
      for (const e of entries) {
        const full = path.join(dir, e.name).replace(/\\/g, '/');
        if (e.isDirectory()) walk(full, depth + 1);
        else if (exts.includes(path.extname(e.name).toLowerCase())) files.push(full);
      }
    };
    walk(path.join(root, 'GameData'), 0);
    this.fileCache.set(key, { at: Date.now(), files });
    return files;
  }

  // Snippet names (with their parameters) defined in this document and component tags it imports.
  documentTags(file: string, text: string): Map<string, { params: string[]; kind: 'snippet' | 'import'; path?: string }> {
    const out = new Map<string, { params: string[]; kind: 'snippet' | 'import'; path?: string }>();
    const scan = scanXml(text);
    const root = gameRootOf(file) ?? path.dirname(file);
    for (const el of scan.all) {
      const attr = (n: string) => el.attrs.find((a) => a.name === n)?.value;
      if (el.name === 'snippet' && attr('name')) out.set(attr('name')!, { params: paramNames(attr('params') ?? ''), kind: 'snippet' });
      if (el.name === 'import' && attr('src')) {
        const target = resolveGamePath(root, file, attr('src')!);
        const body = target ? this.textOf(target) ?? safeRead(target) : undefined;
        if (!body) continue;
        for (const c of scanXml(body).all) {
          if (c.name === 'Component') {
            const tag = c.attrs.find((a) => a.name === 'tag')?.value;
            const params = c.attrs.find((a) => a.name === 'params')?.value ?? '';
            if (tag) out.set(tag, { params: paramNames(params), kind: 'import', path: target });
          }
        }
      }
    }
    return out;
  }
}

export function paramNames(params: string): string[] {
  return params.split(',').map((p) => p.split('=')[0].trim()).filter((p) => p.length > 0);
}

// Paths in markup are relative to the game folder ("GameData/ui/theme.css"); fall back to the file's own folder.
export function resolveGamePath(root: string, fromFile: string, rel: string): string | undefined {
  if (!rel || rel.includes('${')) return undefined;
  const options = [path.join(root, rel), path.join(root, 'GameData', rel), path.join(path.dirname(fromFile), rel)];
  return options.find((o) => fs.existsSync(o))?.replace(/\\/g, '/');
}

export function safeRead(file: string): string | undefined {
  try { return fs.readFileSync(file, 'utf8'); } catch { return undefined; }
}
