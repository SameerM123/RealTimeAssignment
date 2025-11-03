# Se7enAcademy — ITS Prototype (Front‑End Only)

High‑fidelity, front‑end‑only prototype of an Intelligent Tutoring System (ITS) for high‑school science. No backend; deterministic rules, mock JSON data, and local state only.

## Quickstart

```bash
npm i
npm run dev
```

- Build: `npm run build`
- Preview: `npm run preview`

## Routes

- `/` Landing
- `/student` Student Home
- `/student/session` Practice Session (core)
- `/student/plan` Spaced Review Planner
- `/teacher` Teacher Dashboard (core)
- `/concepts` Concept Map
- `/about` About

## Demo Script (3 Walkthroughs)

### A) Detect & Repair a Weak Concept (Student) — ~3 min
1) Go to `/student/session?seed=weakA&mode=guided`  
2) Answer incorrectly or choose low confidence  
3) Basic hint → targeted hint; locked to same concept  
4) After a correct response, mastery +10; progress advances

Success: Sidebar shows Weak badge; explainability tooltip states rule path

### B) Tune Spaced Review Plan (Student) — ~3 min
1) Open `/student/plan`, set intensity = intense  
2) Plan updates deterministically  
3) Hover a plan chip to see reason (e.g., `weak: light_reactions`)

Success: Plan reflects intensity; list counts match strip

### C) Teacher Triage & Export — ~3 min
1) `/teacher` shows Calvin Cycle red cluster (seed `triageB`)  
2) Open first student → see attempts + recommended action  
3) Click Export → downloads `public/sevenacademy-demo.csv`  
4) Acknowledge → item moves/lowers priority

Success: Export present; triage updates; reason codes readable

## Data & Rules

- JSON in `src/data`: `concepts.json`, `items.json`, `student_state.json`, `class_roster.json`, `rules.json`, `seeds.json`
- Core logic in `src/lib/rules.ts` (deterministic; no randomness without seed)
- App state via Zustand slices in `src/state/store.ts`

## Traceability Matrix (skeleton)

| Walkthrough | Use Case IDs | Requirement IDs | Notes |
|---|---|---|---|
| A: Weak Concept Repair | UC‑S1, UC‑S3 | M‑12, S‑05, C‑08 | Mastery update, targeted hint, item selection |
| B: Spaced Review | UC‑S4 | M‑18, S‑09 | Intensity affects schedule deterministically |
| C: Teacher Triage | UC‑T2 | M‑22, S‑14 | Heatmap thresholds, export CSV |

Replace IDs with your spreadsheet’s actual IDs.

## Known Limitations

- No backend, auth, or real ML; all local, deterministic simulation
- shadcn/ui components are not yet installed; Tailwind UI in use
- Some flows are placeholder/skeleton pending polish

## Accessibility

- Keyboard reachable controls; visible focus outlines
- Labels and roles on interactive elements where applicable
- Color contrast targeted at AA with dark theme tokens

## Tech Stack

- React 18 + TypeScript + Vite
- Tailwind CSS
- React Router, Zustand
- Icons: `lucide-react`

## Project Structure

```
/src
  /app          # AppShell, Router
  /pages        # Route components
  /data         # Mock JSON
  /state        # Zustand stores
  /lib          # Rules (deterministic)
  /styles       # Tailwind tokens, global.css
/public
  sevenacademy-demo.csv # Teacher export mock
```

## Deployment

- Local: `npm i && npm run dev`
- Build: `npm run build && npm run preview`
- GitHub Pages/Vercel: standard Vite instructions


