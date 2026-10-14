**Bug report from the photo app team:** "The *Upload all* button says '0 of 3 photos uploaded' immediately, even though the photos *do* show up on the server a moment later. Also, the server log shows all uploads arriving at the same time, and it is rate-limited to one at a time."

`uploadAllFiles` uploads a list of files **one at a time, in order**, and resolves with a report once every upload has finished:

```ts
type UploadReport = { uploaded: string[]; failed: string[] };
function uploadAllFiles(files: readonly string[], upload: (name: string) => Promise<void>): Promise<UploadReport>
```

- The next upload starts only after the previous one has settled.
- Files whose upload resolves go in `uploaded`, files whose upload rejects go in `failed`. A failure doesn't stop the rest.
- Both lists keep the order of `files`.

```ts
await uploadAllFiles(["a.jpg", "b.jpg"], okUpload);
// { uploaded: ["a.jpg", "b.jpg"], failed: [] }

await uploadAllFiles(["a.jpg", "huge.raw", "c.jpg"], uploadThatRejectsHugeFiles);
// { uploaded: ["a.jpg", "c.jpg"], failed: ["huge.raw"] }
```

**Reproduction:**

```ts
await uploadAllFiles(["a.jpg", "b.jpg"], okUpload);
// { uploaded: [], failed: [] }, expected { uploaded: ["a.jpg", "b.jpg"], failed: [] }
```
