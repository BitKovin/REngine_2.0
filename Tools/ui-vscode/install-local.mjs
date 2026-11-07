// Copies the built extension into VS Code's extensions folder (no packaging tools needed). Reload VS Code afterwards.
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const here = path.dirname(fileURLToPath(import.meta.url));
const pkg = JSON.parse(fs.readFileSync(path.join(here, 'package.json'), 'utf8'));
const target = path.join(os.homedir(), '.vscode', 'extensions', `${pkg.publisher}.${pkg.name}-${pkg.version}`);
fs.rmSync(target, { recursive: true, force: true });
fs.mkdirSync(target, { recursive: true });
for (const item of ['package.json', 'dist', 'data', 'syntaxes', 'language-configuration-xml.json', 'language-configuration-css.json']) {
  fs.cpSync(path.join(here, item), path.join(target, item), { recursive: true });
}
console.log('installed to ' + target + '\nReload VS Code (Developer: Reload Window).');
