import {
  createConnection, Diagnostic, DidChangeConfigurationNotification, InitializeParams, ProposedFeatures, TextDocuments,
  TextDocumentSyncKind, CompletionItem, InitializeResult,
} from 'vscode-languageserver/node';
import { TextDocument } from 'vscode-languageserver-textdocument';
import { URI } from 'vscode-uri';
import { colorPresentations, colorsCss, completeCss, cssSymbols, definitionCss, diagnoseCss, hoverCss } from './cssFeatures';
import { colorsXml, completeXml, definitionXml, diagnoseXml, hoverXml, symbolsXml } from './xmlFeatures';
import { Project } from './project';

const connection = createConnection(ProposedFeatures.all);
const documents = new TextDocuments(TextDocument);
const project = new Project();
let canConfigure = false;

project.textOf = (file) => documents.get(URI.file(file).toString())?.getText();

const isCss = (doc: TextDocument) => doc.languageId === 'uicss' || doc.uri.toLowerCase().endsWith('.css');
const pathOf = (doc: TextDocument) => URI.parse(doc.uri).fsPath;

connection.onInitialize((params: InitializeParams): InitializeResult => {
  canConfigure = !!params.capabilities.workspace?.configuration;
  project.bundledDatabase = params.initializationOptions?.bundledDatabase ?? '';
  return {
    capabilities: {
      textDocumentSync: TextDocumentSyncKind.Incremental,
      completionProvider: { triggerCharacters: ['<', ' ', '"', "'", '.', ':', '#', '-', '/', '$', '{', '@', '(', '='], resolveProvider: false },
      hoverProvider: true,
      definitionProvider: true,
      documentSymbolProvider: true,
      colorProvider: true,
    },
  };
});

connection.onInitialized(() => {
  if (canConfigure) void connection.client.register(DidChangeConfigurationNotification.type, undefined);
  void refreshSettings();
});

async function refreshSettings(): Promise<void> {
  if (!canConfigure) return;
  const cfg = (await connection.workspace.getConfiguration('engineUi')) as { databasePath?: string } | null;
  project.databaseOverride = cfg?.databasePath ?? '';
  project.invalidate();
  documents.all().forEach(validate);
}

connection.onDidChangeConfiguration(() => void refreshSettings());
connection.onDidChangeWatchedFiles(() => {
  project.invalidate();
  documents.all().forEach(validate);
});

function guard<T>(fallback: T, fn: () => T): T {
  try {
    return fn();
  } catch (e) {
    connection.console.error(String((e as Error).stack ?? e));
    return fallback;
  }
}

function validate(doc: TextDocument): void {
  const diagnostics: Diagnostic[] = guard<Diagnostic[]>([], () => {
    const db = project.dbFor(pathOf(doc));
    return isCss(doc) ? diagnoseCss(doc, db, project, pathOf(doc)) : diagnoseXml(doc, db, project, pathOf(doc));
  });
  void connection.sendDiagnostics({ uri: doc.uri, diagnostics });
}

let timer: NodeJS.Timeout | undefined;
documents.onDidChangeContent((e) => {
  // a changed style sheet changes class and variable lists everywhere
  if (isCss(e.document)) project.invalidate();
  clearTimeout(timer);
  timer = setTimeout(() => {
    documents.all().forEach(validate);
  }, 250);
});
documents.onDidClose((e) => void connection.sendDiagnostics({ uri: e.document.uri, diagnostics: [] }));

connection.onCompletion((p): CompletionItem[] => {
  const doc = documents.get(p.textDocument.uri);
  if (!doc) return [];
  return guard<CompletionItem[]>([], () => {
    const file = pathOf(doc);
    const db = project.dbFor(file);
    const offset = doc.offsetAt(p.position);
    return isCss(doc) ? completeCss(doc, offset, db, project, file) : completeXml(doc, offset, db, project, file);
  });
});

connection.onHover((p) => {
  const doc = documents.get(p.textDocument.uri);
  if (!doc) return null;
  return guard(null, () => {
    const file = pathOf(doc);
    const db = project.dbFor(file);
    const offset = doc.offsetAt(p.position);
    return isCss(doc) ? hoverCss(doc, offset, db, project, file) : hoverXml(doc, offset, db, project, file);
  });
});

connection.onDefinition((p) => {
  const doc = documents.get(p.textDocument.uri);
  if (!doc) return null;
  return guard(null, () => {
    const file = pathOf(doc);
    const db = project.dbFor(file);
    const offset = doc.offsetAt(p.position);
    return isCss(doc) ? definitionCss(doc, offset, project, file) : definitionXml(doc, offset, db, project, file);
  });
});

connection.onDocumentSymbol((p) => {
  const doc = documents.get(p.textDocument.uri);
  if (!doc) return [];
  return guard([], () => (isCss(doc) ? cssSymbols(doc) : symbolsXml(doc)));
});

connection.onDocumentColor((p) => {
  const doc = documents.get(p.textDocument.uri);
  if (!doc) return [];
  return guard([], () => (isCss(doc) ? colorsCss(doc) : colorsXml(doc, project.dbFor(pathOf(doc)))));
});
connection.onColorPresentation((p) => colorPresentations(p.color));

documents.listen(connection);
connection.listen();
