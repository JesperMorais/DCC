// Bundle Monaco locally (no CDN) and configure its TypeScript service to match the server's checker.
import { loader } from "@monaco-editor/react";
import * as monaco from "monaco-editor";
import EditorWorker from "monaco-editor/editor/editor.worker?worker";
import TsWorker from "monaco-editor/language/typescript/ts.worker?worker";
import { api } from "./api";
import { registerIntellisense } from "./intellisense";

self.MonacoEnvironment = {
  getWorker: (_id: string, label: string) => (label === "typescript" || label === "javascript" ? new TsWorker() : new EditorWorker()),
};

const ts = monaco.typescript;

ts.typescriptDefaults.setCompilerOptions({
  target: ts.ScriptTarget.ES2020,
  lib: ["es2023"],
  strict: true,
  noEmit: true,
  allowNonTsExtensions: true,
  noFallthroughCasesInSwitch: true,
});
ts.typescriptDefaults.setDiagnosticsOptions({ noSemanticValidation: false, noSyntaxValidation: false });
ts.typescriptDefaults.setEagerModelSync(true);

api.harness().then((dts) => ts.typescriptDefaults.addExtraLib(dts, "file:///harness.d.ts"));

export function defineThemes() {
  monaco.editor.defineTheme("daily-dark", {
    base: "vs-dark",
    inherit: true,
    rules: [
      { token: "comment", foreground: "6d7485", fontStyle: "italic" },
      { token: "keyword", foreground: "c678dd" },
      { token: "string", foreground: "7ec699" },
      { token: "number", foreground: "d19a66" },
      { token: "type.identifier", foreground: "6da7ec" },
    ],
    colors: {
      "editor.background": "#0e1116",
      "editor.lineHighlightBackground": "#161a22",
      "editorLineNumber.foreground": "#3a4150",
      "editorLineNumber.activeForeground": "#8a91a2",
      "editor.selectionBackground": "#1f3b63",
      "editorIndentGuide.background1": "#1b2029",
      "editorGutter.background": "#0e1116",
    },
  });
  monaco.editor.defineTheme("daily-light", {
    base: "vs",
    inherit: true,
    rules: [
      { token: "comment", foreground: "858b9b", fontStyle: "italic" },
      { token: "keyword", foreground: "8b36b8" },
      { token: "string", foreground: "1d7d45" },
      { token: "number", foreground: "a65a12" },
      { token: "type.identifier", foreground: "1c5cab" },
    ],
    colors: {
      "editor.background": "#fbfbfc",
      "editor.lineHighlightBackground": "#f1f3f6",
      "editorLineNumber.foreground": "#c3c8d2",
      "editorLineNumber.activeForeground": "#4d5467",
    },
  });
}

loader.config({ monaco });
defineThemes();
registerIntellisense(monaco);

export { monaco };
