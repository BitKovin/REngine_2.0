import * as path from 'path';
import { workspace, ExtensionContext } from 'vscode';
import { LanguageClient, LanguageClientOptions, ServerOptions, TransportKind } from 'vscode-languageclient/node';

let client: LanguageClient | undefined;

export function activate(context: ExtensionContext): void {
  const server = context.asAbsolutePath(path.join('dist', 'server.js'));
  const serverOptions: ServerOptions = {
    run: { module: server, transport: TransportKind.ipc },
    debug: { module: server, transport: TransportKind.ipc, options: { execArgv: ['--nolazy', '--inspect=6011'] } },
  };
  const clientOptions: LanguageClientOptions = {
    documentSelector: [{ language: 'uixml' }, { language: 'uicss' }],
    synchronize: { fileEvents: workspace.createFileSystemWatcher('**/*.{xml,css,json}') },
    initializationOptions: { bundledDatabase: context.asAbsolutePath(path.join('data', 'uidb.json')) },
  };
  client = new LanguageClient('engineUi', 'Engine UI Language', serverOptions, clientOptions);
  void client.start();
}

export function deactivate(): Thenable<void> | undefined {
  return client?.stop();
}
