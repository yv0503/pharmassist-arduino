#ifndef HTML_CONTENT_H
#define HTML_CONTENT_H

const char* HTML_MAIN_CONTENT = R"(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>PharmAssist</title>
<style>
body{font-family:sans-serif;background:#121212;height:100vh;margin:0;display:flex;justify-content:center;align-items:center;color:#e0e0e0}
.c{background:#1e1e1e;border-radius:20px;padding:2rem;box-shadow:0 10px 30px rgba(0,0,0,.3);text-align:center;max-width:600px;width:90%}
h1{color:#64b5f6;margin-bottom:.5rem;font-size:2.5rem}
h2{color:#9e9e9e;font-weight:normal;margin-bottom:2rem;font-size:1.2rem}
.rb{background:#f44336;color:#fff;border:none;padding:.8rem 2rem;border-radius:50px;font-size:1rem;cursor:pointer;transition:.3s;margin-top:2rem}
.rb:hover{background:#d32f2f;transform:translateY(-2px);box-shadow:0 5px 15px rgba(244,67,54,.3)}
.m{display:none;position:fixed;z-index:1;left:0;top:0;width:100%;height:100%;background:rgba(0,0,0,.7);align-items:center;justify-content:center}
.mc{background:#252525;padding:1.5rem;border-radius:15px;text-align:center;max-width:500px;width:90%}
.wi{color:#ffb300;font-size:3rem;margin-bottom:1rem}
.bg{display:flex;justify-content:center;gap:1rem;margin-top:1.5rem}
.cb,.cfb{color:#fff;border:none;padding:.6rem 1.5rem;border-radius:50px;cursor:pointer;transition:.3s}
.cb{background:#616161}
.cfb{background:#f44336}
.cb:hover,.cfb:hover{transform:translateY(-2px);box-shadow:0 5px 15px rgba(0,0,0,.2)}
footer{margin-top:2rem;color:#9e9e9e;font-size:.9rem}
</style>
</head>
<body>
<div class="c">
<h1>Hello World from PharmAssist!</h1>
<h2>This is a simple web server running on Arduino UNO R4 WiFi.</h2>
<button class="rb" id="b">Factory Reset Device</button>
<footer>&copy; 2025 PharmAssist</footer>
</div>
<div class="m" id="m">
<div class="mc">
<div class="wi">⚠️</div>
<h2>Warning!</h2>
<p>You are about to factory reset this device. All data and settings will be permanently erased.</p>
<p>Are you sure you want to continue?</p>
<div class="bg">
<button class="cb" id="c">Cancel</button>
<button class="cfb" id="f">Reset Device</button>
</div>
</div>
</div>
<script>
let b=document.getElementById('b'),
m=document.getElementById('m'),
c=document.getElementById('c'),
f=document.getElementById('f');
b.onclick=function(){m.style.display='flex'};
c.onclick=function(){m.style.display='none'};
f.onclick=function(){window.location.href='/reset'};
window.onclick=function(e){if(e.target===m)m.style.display='none'};
</script>
</body>
</html>
)";

const char* HTML_RESET_CONTENT = R"(

<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>PharmAssist Reset</title>
<style>
body{font-family:sans-serif;background:#121212;height:100vh;margin:0;display:flex;justify-content:center;align-items:center;color:#e0e0e0}
.c{background:#1e1e1e;border-radius:20px;padding:2rem;box-shadow:0 10px 30px rgba(0,0,0,.3);text-align:center;max-width:600px;width:90%}
h1{color:#64b5f6;margin-bottom:1rem;font-size:2.5rem}
.icon{font-size:4rem;color:#4caf50;margin-bottom:1rem}
p{color:#bdbdbd;font-size:1.1rem;line-height:1.5;margin-bottom:1rem}
.note{color:#ffb74d;font-size:1rem;border:1px solid #ffb74d;border-radius:10px;padding:1rem;margin:1.5rem 0}
.back{background:#616161;color:#fff;border:none;padding:.8rem 2rem;border-radius:50px;font-size:1rem;cursor:pointer;transition:.3s;text-decoration:none;display:inline-block;margin-top:1rem}
.back:hover{background:#757575;transform:translateY(-2px);box-shadow:0 5px 15px rgba(0,0,0,.2)}
footer{margin-top:2rem;color:#9e9e9e;font-size:.9rem}
</style>
</head>
<body>
<div class="c">
<div class="icon">✓</div>
<h1>Reset Complete</h1>
<p>Your PharmAssist device has been successfully returned to factory settings.</p>
<div class="note">
<strong>Important:</strong> The device must be manually restarted to complete the process.
</div>
<p>Please power cycle your PharmAssist device by disconnecting and reconnecting the power source.</p>
<a href="/" class="back">Back to Home</a>
<footer>&copy; 2025 PharmAssist</footer>
</div>\
</body>
</html>
)";

#endif
