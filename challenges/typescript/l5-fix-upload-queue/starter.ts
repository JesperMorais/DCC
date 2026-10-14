type UploadReport = { uploaded: string[]; failed: string[] };

async function uploadAllFiles(files: readonly string[], upload: (name: string) => Promise<void>): Promise<UploadReport> {
  const report: UploadReport = { uploaded: [], failed: [] };
  files.forEach(async (name) => {
    try {
      await upload(name);
      report.uploaded.push(name);
    } catch {
      report.failed.push(name);
    }
  });
  return report;
}
