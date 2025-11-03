export function AboutPage() {
  return (
    <div className="prose prose-invert max-w-none">
      <h2>About</h2>
      <p>
        This prototype demonstrates an Intelligent Tutoring System (ITS) for high-school science. It focuses on
        deterministic rules to simulate adaptivity, explainability, and spaced review — without any backend.
      </p>
      <ul>
        <li><strong>Domain</strong>: Concepts, items, misconceptions</li>
        <li><strong>Student</strong>: Mastery per concept, attempt history</li>
        <li><strong>Tutoring</strong>: Rules for grading, selection, hints</li>
        <li><strong>UI</strong>: Accessible, explainable, repeatable demo flows</li>
      </ul>
    </div>
  )
}


