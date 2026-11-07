// Starts the built server over stdio and checks real answers. Run: npm test
import { spawn } from 'node:child_process';
import path from 'node:path';
import fs from 'node:fs';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { createRequire } from 'node:module';
const require = createRequire(import.meta.url);
const rpc = require('vscode-jsonrpc/node');

const here = path.dirname(fileURLToPath(import.meta.url));
const root = path.resolve(here, '..');
const game = path.resolve(root, '../../source/Games/StealthAction');
const proc = spawn(process.execPath, [path.join(root, 'dist/server.js'), '--stdio'], { stdio: ['pipe', 'pipe', 'inherit'] });
const conn = rpc.createMessageConnection(new rpc.StreamMessageReader(proc.stdout), new rpc.StreamMessageWriter(proc.stdin));
const diags = new Map();
conn.onNotification('textDocument/publishDiagnostics', (p) => diags.set(p.uri, p.diagnostics));
conn.onRequest('workspace/configuration', () => [{}]);
conn.onRequest('client/registerCapability', () => null);
conn.listen();

let failed = 0;
const check = (name, ok, extra = '') => {
  console.log((ok ? 'ok   ' : 'FAIL ') + name + (ok ? '' : '  ' + extra));
  if (!ok) failed++;
};
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));

await conn.sendRequest('initialize', {
  processId: process.pid,
  rootUri: null,
  capabilities: { workspace: { configuration: true } },
  initializationOptions: { bundledDatabase: path.join(root, 'data/uidb.json') },
});
conn.sendNotification('initialized', {});

let version = 1;
async function open(file, text, lang) {
  // every call gets its own document name, so diagnostics of earlier texts never mix in
  const uri = pathToFileURL(file.replace(/(\.\w+)$/, '_' + version + '$1')).toString();
  conn.sendNotification('textDocument/didOpen', { textDocument: { uri, languageId: lang, version: version++, text } });
  return uri;
}

// text with | marking the cursor
function cursor(text) {
  const i = text.indexOf('|');
  const before = text.slice(0, i);
  const lines = before.split('\n');
  return { text: text.replace('|', ''), pos: { line: lines.length - 1, character: lines[lines.length - 1].length } };
}
async function complete(file, lang, marked) {
  const { text, pos } = cursor(marked);
  const uri = await open(file, text, lang);
  const r = await conn.sendRequest('textDocument/completion', { textDocument: { uri }, position: pos });
  return (Array.isArray(r) ? r : r?.items ?? []).map((i) => i.label);
}
async function diagnose(file, lang, text) {
  const uri = await open(file, text, lang);
  for (let i = 0; i < 50 && !diags.has(uri); i++) await sleep(100);
  return diags.get(uri) ?? [];
}

const xmlFile = path.join(game, 'GameData/ui/kit/pages/_t1.xml');
const cssFile = path.join(game, 'GameData/ui/_t1.css');

// --- XML completion
let l = await complete(xmlFile, 'uixml', '<root>\n<Pan|');
check('tag name completes Panel', l.includes('Panel') && l.includes('Label'), l.slice(0, 5));
l = await complete(xmlFile, 'uixml', '<root>\n<Panel cl|');
check('attribute name completes class', l.includes('class') && l.includes('visible'), l.slice(0, 5));
l = await complete(xmlFile, 'uixml', '<root>\n<Panel visible="|"/>');
check('bool value', l.includes('true') && l.includes('false'), l);
l = await complete(xmlFile, 'uixml', '<root>\n<Panel class="kit-|"/>');
check('class names from style sheets', l.includes('kit-screen') && l.includes('kit-column'), l.slice(0, 8));
l = await complete(xmlFile, 'uixml', '<root>\n<TextButton onactivate="Se|"/>');
check('action names', l.includes('SelectTab') || l.includes('Back'), l.slice(0, 8));
l = await complete(xmlFile, 'uixml', '<root>\n<Label text="${SETT|}"/>');
check('locale keys in dollar-brace', l.some((k) => k.startsWith('SETTINGS')), l.slice(0, 5));
l = await complete(xmlFile, 'uixml', '<root>\n<Panel style="wi|"/>');
check('css property inside style attribute', l.includes('width'), l.slice(0, 5));
l = await complete(xmlFile, 'uixml', '<root>\n<styles><include src="|"/></styles>');
check('include src lists style sheets', l.some((k) => k.endsWith('theme.css')), l.slice(0, 3));
l = await complete(xmlFile, 'uixml', '<root>\n<Panel>\n</|');
check('close tag', l.includes('Panel'), l);

// --- XML diagnostics
let d = await diagnose(xmlFile, 'uixml', '<root>\n<Panel colour="x" visible="maybe"><Lable/></Panel>\n</root>');
const msgs = d.map((x) => x.message).join(' | ');
check('unknown attribute reported', /no attribute 'colour'/.test(msgs), msgs);
check('bad bool reported', /true or false/.test(msgs), msgs);
check('unknown tag reported with suggestion', /Unknown tag 'Lable'.*Label/.test(msgs), msgs);
d = await diagnose(xmlFile, 'uixml', '<root>\n<Panel>\n</root>');
check('unclosed tag reported', d.some((x) => /not closed/.test(x.message)), JSON.stringify(d));

for (const rel of ['GameData/ui/kit/pages/settings.xml', 'GameData/ui/kit/pages/confirm.xml', 'GameData/ui/kit/pages/video.xml', 'GameData/ui/kit/pages/input.xml']) {
  const f = path.join(game, rel);
  if (!fs.existsSync(f)) continue;
  d = await diagnose(f, 'uixml', fs.readFileSync(f, 'utf8'));
  const errs = d.filter((x) => x.severity === 1);
  check('no errors in ' + rel, errs.length === 0, errs.map((e) => e.message).join(' | '));
}

// --- CSS
l = await complete(cssFile, 'uicss', '.a { wid| }');
check('css property completes', l.includes('width'), l.slice(0, 5));
l = await complete(cssFile, 'uicss', '.a { flow-children: | }');
check('css value keywords', l.includes('down') || l.includes('right'), l.slice(0, 8));
l = await complete(cssFile, 'uicss', '.a { color: var(--kit-|) }');
check('var() names', l.includes('--kit-text'), l.slice(0, 5));
l = await complete(cssFile, 'uicss', '.a:ho| { }');
check('pseudo class', l.includes('hover'), l);
l = await complete(cssFile, 'uicss', 'Pan| { }');
check('type selector', l.includes('Panel'), l.slice(0, 5));
l = await complete(cssFile, 'uicss', '.a { transition-timing-function: | }');
check('easing values', l.includes('ease-in'), l.slice(0, 5));
l = await complete(cssFile, 'uicss', '@im|');
check('at-rule', l.includes('@import'), l);

d = await diagnose(cssFile, 'uicss', '.a { widht: 10px; color: var(--nope); }\n.b:hovr { }\n.c { width: 1px');
const cm = d.map((x) => x.message).join(' | ');
check('css unknown property', /Unknown property 'widht'.*width/.test(cm), cm);
check('css undefined variable', /--nope/.test(cm), cm);
check('css unknown state', /hovr/.test(cm), cm);
check('css missing brace', /closing/.test(cm), cm);
for (const rel of ['GameData/ui/theme.css', 'GameData/ui/kit/styles/kit.css']) {
  const f = path.join(game, rel);
  d = await diagnose(f, 'uicss', fs.readFileSync(f, 'utf8'));
  const errs = d.filter((x) => x.severity <= 2);
  check('no problems in ' + rel, errs.length === 0, errs.slice(0, 3).map((e) => e.message).join(' | '));
}

// hover, colours, symbols
{
  const { text, pos } = cursor('.a { wid|th: 10px; background-color: #ff0000; }');
  const uri = await open(cssFile, text, 'uicss');
  const h = await conn.sendRequest('textDocument/hover', { textDocument: { uri }, position: pos });
  check('css hover shows grammar', /width/.test(JSON.stringify(h)), JSON.stringify(h));
  const colors = await conn.sendRequest('textDocument/documentColor', { textDocument: { uri } });
  check('colour found', colors.length === 1 && colors[0].color.red === 1, JSON.stringify(colors));
}
{
  const { text, pos } = cursor('<root>\n<Panel visi|ble="true"/>');
  const uri = await open(xmlFile, text, 'uixml');
  const h = await conn.sendRequest('textDocument/hover', { textDocument: { uri }, position: pos });
  check('xml attribute hover', /visible/.test(JSON.stringify(h)) && /bool/.test(JSON.stringify(h)), JSON.stringify(h));
  const syms = await conn.sendRequest('textDocument/documentSymbol', { textDocument: { uri } });
  check('xml symbols', syms.length === 1 && syms[0].name === 'root', JSON.stringify(syms));
}

if (process.env.CORPUS) {
  // every markup and style file of the game: lists what the checker complains about (to find false alarms)
  const walk = (dir) => fs.readdirSync(dir, { withFileTypes: true }).flatMap((e) => e.isDirectory() ? walk(path.join(dir, e.name)) : [path.join(dir, e.name)]);
  const counts = new Map();
  for (const f of walk(path.join(game, 'GameData/ui')).filter((f) => /\.(xml|css)$/.test(f))) {
    const dd = await diagnose(f, f.endsWith('.css') ? 'uicss' : 'uixml', fs.readFileSync(f, 'utf8'));
    for (const x of dd) {
      const key = x.severity + ' ' + x.message.replace(/'[^']*'/g, "'_'") + '  e.g. ' + path.basename(f) + ':' + (x.range.start.line + 1);
      const k = key.split('  e.g.')[0];
      if (!counts.has(k)) counts.set(k, key);
    }
  }
  console.log('--- corpus findings (one example each)');
  for (const v of counts.values()) console.log(v);
}
await conn.sendRequest('shutdown');
conn.sendNotification('exit');
console.log(failed ? `\n${failed} FAILED` : '\nall passed');
process.exit(failed ? 1 : 0);
