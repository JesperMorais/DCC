import { Mascot, TessSays, type Mood } from "../components/Mascot";

const MOODS: Mood[] = ["idle", "wave", "happy", "cheer", "think", "sleep", "sad"];

export default function MascotGallery() {
  return (
    <div className="mx-auto max-w-[1100px] px-8 py-10">
      <h1 className="text-[28px] font-semibold tracking-tight">Meet Tess</h1>
      <p className="mt-1 text-sm text-ink-2">The daily.ts pangolin. Scales are armour, and so are types.</p>
      <div className="mt-8 grid grid-cols-4 gap-5">
        {MOODS.map((m) => (
          <div key={m} className="card flex flex-col items-center p-5">
            <Mascot mood={m} size={180} className="text-ink-2" />
            <span className="mt-2 font-mono text-xs text-muted">{m}</span>
          </div>
        ))}
      </div>
      <div className="mt-8 flex gap-8">
        <TessSays mood="wave">Hi! I'm Tess. One challenge a day keeps the bugs away.</TessSays>
        <Mascot mood="idle" size={48} />
        <Mascot mood="happy" size={32} />
      </div>
    </div>
  );
}
