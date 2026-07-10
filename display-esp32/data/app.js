// BeeTap Control - vanilla JS wiring for the real device API.
// Reimplements the "BeeTap Control.dc.html" Claude Design mockup
// (project e4750120-6cf9-463d-8540-04c588b6e35e) against the actual
// display-esp32 firmware endpoints instead of its fake demo state.

const GAUGE_RADIUS = 86;
const GAUGE_CIRCUMFERENCE = 2 * Math.PI * GAUGE_RADIUS;
const WEIGHT_POLL_MS = 2000;

let toastTimer = null;

function showToast(msg) {
  const el = document.getElementById('toast');
  el.textContent = msg;
  el.hidden = false;
  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => { el.hidden = true; }, 2400);
}

function setTab(tab) {
  document.getElementById('tab-home').dataset.on = String(tab === 'home');
  document.getElementById('tab-settings').dataset.on = String(tab === 'settings');
  document.getElementById('panel-home').hidden = tab !== 'home';
  document.getElementById('panel-settings').hidden = tab !== 'settings';
}

function levelColor(level) {
  if (level >= 0.66) return 'var(--level-full)';
  if (level >= 0.50) return 'var(--level-high)';
  if (level >= 0.33) return 'var(--level-mid)';
  if (level >= 0.15) return 'var(--level-low)';
  return 'var(--level-empty)';
}

function updateGauge(weightKg, capacityKg, connected) {
  const level = connected ? Math.max(0, Math.min(1, weightKg / (capacityKg || 19))) : 0;
  const arc = document.getElementById('gauge-arc');
  arc.style.stroke = levelColor(level);
  arc.setAttribute('stroke-dashoffset', String(GAUGE_CIRCUMFERENCE * (1 - level)));
  document.getElementById('weight-value').textContent = connected ? weightKg.toFixed(2) : '--.--';
}

function updateStatusBadges(connected, stale) {
  let tone = 'full', label = 'Live';
  if (!connected) { tone = 'empty'; label = 'Offline'; }
  else if (stale) { tone = 'low'; label = 'Stale'; }

  for (const el of document.querySelectorAll('.js-status-badge')) {
    el.dataset.tone = tone;
    el.querySelector('.label').textContent = label;
  }
}

let lastCapacityKg = 19;

async function pollScale() {
  try {
    const [weightRes, statusRes] = await Promise.all([fetch('/weight'), fetch('/scale_status')]);
    const weightKg = parseFloat(await weightRes.text()) || 0;
    const status = await statusRes.json();
    updateStatusBadges(status.connected, status.stale);
    updateGauge(weightKg, lastCapacityKg, status.connected);
  } catch (e) {
    updateStatusBadges(false, false);
  }
}

async function loadBeerProfile() {
  try {
    const res = await fetch('/beer_profile');
    const profile = await res.json();
    document.getElementById('beer-name').value = profile.name ?? '';
    document.getElementById('beer-sg').value = profile.sg ?? '';
    document.getElementById('beer-ibu').value = profile.ibu ?? '';
    document.getElementById('beer-ebc').value = profile.ebc ?? '';
    document.getElementById('beer-abv').value = profile.abv ?? '';
    document.getElementById('capacity').value = profile.capacity ?? 19;
    lastCapacityKg = Number(profile.capacity) || 19;
  } catch (e) {
    showToast('Could not load beer profile');
  }
}

function postForm(path, params) {
  const body = new URLSearchParams(params);
  return fetch(path, { method: 'POST', body });
}

async function saveBeerProfile() {
  const name = document.getElementById('beer-name').value;
  const sg = document.getElementById('beer-sg').value;
  const ibu = document.getElementById('beer-ibu').value;
  const ebc = document.getElementById('beer-ebc').value;
  const abv = document.getElementById('beer-abv').value;
  try {
    await Promise.all([
      postForm('/beer_name', { beer_name: name }),
      postForm('/beer_sg', { beer_sg: sg }),
      postForm('/beer_ibu', { beer_ibu: ibu }),
      postForm('/beer_ebc', { beer_ebc: ebc }),
      postForm('/beer_abv', { beer_abv: abv }),
    ]);
    showToast('Beer profile saved');
  } catch (e) {
    showToast('Failed to save beer profile');
  }
}

async function saveCapacity() {
  const capacity = document.getElementById('capacity').value;
  lastCapacityKg = Number(capacity) || 19;
  try {
    await postForm('/beer_capacity', { beer_capacity: capacity });
  } catch (e) {
    showToast('Failed to save capacity');
  }
}

async function doTare() {
  try {
    await fetch('/tare', { method: 'POST' });
    showToast('Tare captured - scale zeroed');
  } catch (e) {
    showToast('Tare command failed');
  }
}

async function doRestart() {
  showToast('Restarting BeeTap display...');
  try { await fetch('/restart', { method: 'POST' }); } catch (e) { /* expected: device restarts mid-response */ }
}

async function doResetWifi() {
  showToast('Wifi settings cleared - device will boot into AP setup mode');
  try { await fetch('/reset_wifi', { method: 'POST' }); } catch (e) { /* expected: device restarts mid-response */ }
}

function uploadOta(file, mode, onProgress) {
  return fetch(`/ota/start?mode=${mode}`)
    .then((startRes) => {
      if (!startRes.ok) return startRes.text().then((t) => { throw new Error(t || 'Failed to start update'); });
      return new Promise((resolve, reject) => {
        const xhr = new XMLHttpRequest();
        xhr.open('POST', '/ota/upload');
        xhr.upload.onprogress = (e) => { if (e.lengthComputable) onProgress(e.loaded / e.total); };
        xhr.onload = () => (xhr.status === 200 ? resolve() : reject(new Error(xhr.responseText || 'Upload failed')));
        xhr.onerror = () => reject(new Error('Network error during upload'));
        const formData = new FormData();
        formData.append('file', file, file.name);
        xhr.send(formData);
      });
    });
}

function setupUploadCard(prefix, mode, successMsg) {
  const fileInput = document.getElementById(`${prefix}-file`);
  const nameEl = document.getElementById(`${prefix}-file-name`);
  const button = document.getElementById(`${prefix}-upload-btn`);
  const progressWrap = document.getElementById(`${prefix}-progress-wrap`);
  const progressFill = document.getElementById(`${prefix}-progress-fill`);
  const progressPct = document.getElementById(`${prefix}-progress-pct`);
  let selectedFile = null;

  fileInput.addEventListener('change', () => {
    selectedFile = fileInput.files && fileInput.files[0] ? fileInput.files[0] : null;
    nameEl.textContent = selectedFile ? selectedFile.name : 'Choose a .bin file...';
    nameEl.style.color = selectedFile ? 'var(--text-primary)' : 'var(--text-muted)';
  });

  button.addEventListener('click', async () => {
    if (!selectedFile) { showToast('Choose a file first'); return; }
    button.disabled = true;
    progressWrap.hidden = false;
    progressFill.style.width = '0%';
    progressPct.textContent = '0%';
    button.textContent = 'Uploading...';
    try {
      await uploadOta(selectedFile, mode, (frac) => {
        const pct = Math.round(frac * 100);
        progressFill.style.width = `${pct}%`;
        progressPct.textContent = `${pct}%`;
      });
      showToast(successMsg);
      fileInput.value = '';
      selectedFile = null;
      nameEl.textContent = 'Choose a .bin file...';
      nameEl.style.color = 'var(--text-muted)';
    } catch (e) {
      showToast(e.message || 'Upload failed');
    } finally {
      button.disabled = false;
      progressWrap.hidden = true;
      button.textContent = prefix === 'ota' ? 'Upload & install' : 'Upload filesystem';
    }
  });
}

function init() {
  document.getElementById('tab-home').addEventListener('click', () => setTab('home'));
  document.getElementById('tab-settings').addEventListener('click', () => setTab('settings'));

  document.getElementById('tare-btn').addEventListener('click', doTare);
  document.getElementById('capacity').addEventListener('change', saveCapacity);
  document.getElementById('save-profile-btn').addEventListener('click', saveBeerProfile);
  document.getElementById('restart-btn').addEventListener('click', doRestart);
  document.getElementById('reset-wifi-btn').addEventListener('click', doResetWifi);

  setupUploadCard('ota', 'firmware', 'Firmware installed - rebooting BeeTap display');
  setupUploadCard('spiffs', 'fs', 'Filesystem image installed - reloading web UI');

  loadBeerProfile();
  pollScale();
  setInterval(pollScale, WEIGHT_POLL_MS);
}

document.addEventListener('DOMContentLoaded', init);
