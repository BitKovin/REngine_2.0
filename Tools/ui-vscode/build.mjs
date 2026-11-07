import { build, context } from 'esbuild';

const common = { bundle: true, platform: 'node', target: 'node18', sourcemap: true, logLevel: 'info' };
const jobs = [
  { ...common, entryPoints: ['client/src/extension.ts'], outfile: 'dist/client.js', external: ['vscode'], format: 'cjs' },
  { ...common, entryPoints: ['server/src/server.ts'], outfile: 'dist/server.js', format: 'cjs' },
];
if (process.argv.includes('--watch')) {
  for (const j of jobs) await (await context(j)).watch();
} else {
  for (const j of jobs) await build(j);
}
