#ifndef HTML_CONTENT_H
#define HTML_CONTENT_H

inline auto HTML_MAIN_CONTENT = R"(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>PharmAssist</title>
<style>
body{font-family:system-ui,-apple-system,sans-serif;background:#0f172a;margin:0;height:100vh;display:flex;justify-content:center;align-items:center;color:#e2e8f0;overflow:hidden}
.c{position:relative;background:#1e293b;border-radius:24px;padding:2rem;box-shadow:0 25px 50px -12px rgba(0,0,0,.5);text-align:center;max-width:600px;width:90%;overflow:hidden;z-index:1}
.c::before{content:"";position:absolute;top:-50%;left:-50%;width:200%;height:200%;background:linear-gradient(45deg,#3b82f6,#10b981,#8b5cf6);opacity:.1;animation:r 8s linear infinite;z-index:-1}
@keyframes r{0%{transform:rotate(0deg)}100%{transform:rotate(360deg)}}
.c::after{content:"";position:absolute;inset:0;background:#1e293b;border-radius:24px;z-index:-1}
h1{color:#38bdf8;margin-bottom:.5rem;font-size:2.5rem;font-weight:700;text-shadow:0 0 30px rgba(56,189,248,.5);letter-spacing:-.02em}
h2{color:#94a3b8;font-weight:400;margin-bottom:1.5rem;font-size:1.1rem;max-width:90%;margin-left:auto;margin-right:auto}
.i{display:flex;justify-content:center;margin:2rem 0}
.i svg{width:120px;height:120px;filter:drop-shadow(0 0 10px rgba(56,189,248,.3))}
.i circle{fill:#0f172a;stroke:#38bdf8;stroke-width:2}
.i path{fill:#38bdf8}
.g{position:absolute;top:0;left:0;right:0;height:6px;background:linear-gradient(90deg,#3b82f6,#8b5cf6,#10b981);border-top-left-radius:24px;border-top-right-radius:24px}
footer{margin-top:2rem;color:#64748b;font-size:.8rem}
</style>
</head>
<body>
<div class="c">
<div class="g"></div>
<div class="i">
<svg viewBox="0 0 100 100">
<circle cx="50" cy="50" r="46"/>
<path d="M50,20c-16.5,0-30,13.5-30,30s13.5,30,30,30s30-13.5,30-30S66.5,20,50,20z M36,42h28c1.1,0,2,0.9,2,2s-0.9,2-2,2H36c-1.1,0-2-0.9-2-2
S34.9,42,36,42z M36,55h28c1.1,0,2,0.9,2,2s-0.9,2-2,2H36c-1.1,0-2-0.9-2-2S34.9,55,36,55z M36,68h28c1.1,0,2,0.9,2,2s-0.9,2-2,2H36
c-1.1,0-2-0.9-2-2S34.9,68,36,68z"/>
</svg>
</div>
<h1>Hello World from PharmAssist!</h1>
<h2>This is a simple web server running on Arduino UNO R4 WiFi.</h2>
<footer>&copy; 2025 PharmAssist</footer>
</div>
</body>
</html>
)";

#endif
