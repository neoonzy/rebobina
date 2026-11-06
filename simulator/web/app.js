/* SPDX-License-Identifier: GPL-2.0-only */
/* Interface visual; estados e ações autoritativos são calculados pelo C. */
(() => {
  "use strict";
  const rows = document.getElementById("lcd-rows");
  const stateLabel = document.getElementById("state-indicator");
  const connection = document.getElementById("connection");
  const pageLabel = document.getElementById("metric-page");
  const moduleLabel = document.getElementById("metric-module");
  const eventCount = document.getElementById("metric-events");
  const hint = document.getElementById("lcd-hint");
  const kicker = document.getElementById("screen-kicker");
  const pageNames = ["MENU", "INÍCIO", "FERRAMENTAS", "MÓDULOS", "SOBRE"];
  let events = "";
  let pending = Promise.resolve();

  async function refresh(nextEvents) {
    const result = await fetch("/api/state?events=" + encodeURIComponent(nextEvents), {cache: "no-store"});
    if (!result.ok) throw new Error("HTTP " + result.status);
    const payload = await result.json();
    if (!Array.isArray(payload.rows)) throw new Error("Resposta inválida do núcleo");
    events = nextEvents;
    rows.replaceChildren();
    payload.rows.forEach((row, index) => {
      const line = document.createElement("div");
      line.className = "lcd-row";
      if (row.selected) line.classList.add("selected");
      if (index === 0 && payload.state.opened) line.dataset.header = "true";
      line.textContent = row.text;
      rows.appendChild(line);
    });
    const st = payload.state;
    const page = pageNames[st.page] || "DESCONHECIDO";
    stateLabel.textContent = st.opened ? "ATIVO" : "ESPERA";
    kicker.textContent = st.page ? "SIM / " + page : st.opened ? "SIM / MENU" : "SIM / PRONTO";
    hint.textContent = st.page ? "VOLTAR: MENU" : "TRASH: " + (st.opened ? "FECHAR" : "ABRIR");
    pageLabel.textContent = st.opened ? page : "FECHADO";
    moduleLabel.textContent = st.hello_active ? "HABILITADO NA SIMULAÇÃO" : "DESABILITADO";
    eventCount.textContent = String(events.length);
    connection.textContent = "NÚCLEO C / RESPONDENDO";
  }

  function send(symbol) {
    pending = pending.catch(() => {}).then(async () => {
      if (events.length >= 128) {
        connection.textContent = "LIMITE ATINGIDO / REINICIE";
        return;
      }
      try {
        await refresh(events + symbol);
      } catch (err) {
        connection.textContent = "ERRO DE COMUNICAÇÃO: " + err.message;
      }
    });
  }

  document.querySelectorAll("[data-event]").forEach(button => {
    button.addEventListener("click", () => send(button.dataset.event));
  });
  document.getElementById("reset").addEventListener("click", () => {
    pending = pending.catch(() => {}).then(async () => {
      try { await refresh(""); }
      catch (err) { connection.textContent = "ERRO DE REINÍCIO: " + err.message; }
    });
  });
  document.addEventListener("keydown", event => {
    if (event.altKey || event.metaKey || event.ctrlKey) return;
    if (event.target && /INPUT|TEXTAREA|SELECT/.test(event.target.tagName)) return;
    const mapping = {ArrowUp:"u",ArrowDown:"d",Enter:"s",Escape:"b",Backspace:"b",Delete:"t"};
    const key = mapping[event.key] || (event.key.toLowerCase() === "t" ? "t" : null);
    if (!key) return;
    event.preventDefault();
    send(key);
  });
  refresh("").catch(err => {
    connection.textContent = "NÚCLEO C INDISPONÍVEL: " + err.message;
  });
})();
