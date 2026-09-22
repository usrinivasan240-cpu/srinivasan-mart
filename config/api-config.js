// ============================================================
// Sri Mart — API endpoint config for static hosting (e.g. Vercel).
//
// The C++ backend cannot run on Vercel, so when these pages are served
// statically they must point at a backend hosted elsewhere (see README
// "Deploy": Render/Fly + Dockerfile). Resolution order:
//   1. localStorage "sri_api_base" (per-browser override, handy for testing)
//   2. window.SRI_API_BASE below (edit this one line after deploying the API)
//   3. window.location.origin (local dev: C++ server serves these pages itself)
// ============================================================

window.SRI_API_BASE = "https://srinivasan-mart.onrender.com";

function sriApiBase() {
  try {
    var override = localStorage.getItem("sri_api_base");
    if (override) return override;
  } catch (e) {
    /* storage unavailable: fall through */
  }
  if (window.SRI_API_BASE) return window.SRI_API_BASE;
  return window.location.origin;
}
