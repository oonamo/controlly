// _static/custom.js
document.addEventListener("DOMContentLoaded", function() {
  const paramDescriptions = document.querySelectorAll("dl.field-list dd p, dl.field-list dd ul li p");

  paramDescriptions.forEach(p => {
    let html = p.innerHTML;

    // Injecting an icon span along with the text
    html = html.replace(/\[in\]/g, '<span class="dir-badge"><span class="dir-icon">→</span> in</span>');
    html = html.replace(/\[out\]/g, '<span class="dir-badge"><span class="dir-icon">←</span> out</span>');
    html = html.replace(/\[inout\]/g, '<span class="dir-badge"><span class="dir-icon">↔</span> in, out</span>');
    html = html.replace(/\[in,out\]/g, '<span class="dir-badge"><span class="dir-icon">↔</span> in, out</span>');

    p.innerHTML = html;
  });
});
