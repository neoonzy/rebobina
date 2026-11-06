/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Ponte deterministica (CLI) entre menu C real e simulador visual.
 * Uso: ./build/rebobina-sim-bridge [sequencia t/u/d/s/b]
 * stdout e um protocolo de texto interno; NAO acessa hardware ou discos.
 */
#include "rebobina/manager.h"
#include "rebobina/ui.h"
#include <stdio.h>
#include <string.h>

#define RB_SIM_MAX_EVENTS 128u

static const rb_host_api *rb_demo_host;
static rb_status rb_demo_start(const rb_host_api *host)
{
    rb_demo_host = host;
    host->log("hello", "Iniciado apenas no simulador");
    return RB_OK;
}
static void rb_demo_stop(void)
{
    if (rb_demo_host)
        rb_demo_host->log("hello", "Encerrado no simulador");
    rb_demo_host = NULL;
}
static void rb_log(const char *id, const char *message)
{
    fprintf(stderr, "[sim/%s] %s\n", id, message);
}
static const rb_module rb_demo_module = {
    RB_MODULE_ABI_VERSION, "hello", "Hello (simulacao)",
    rb_demo_start, rb_demo_stop
};

static void emit_row(void *ctx, unsigned row, const char *text, int selected)
{
    (void)ctx;
    printf("ROW\t%u\t%d\t%s\n", row, selected, text);
}
static rb_ui_button decode(char symbol)
{
    switch (symbol) {
    case 't': return RB_UI_TRASH;
    case 'u': return RB_UI_UP;
    case 'd': return RB_UI_DOWN;
    case 's': return RB_UI_SET;
    case 'b': return RB_UI_BACK;
    default: return (rb_ui_button)99;
    }
}

int main(int argc, char **argv)
{
    const char *events = argc == 2 ? argv[1] : "";
    rb_ui ui;
    rb_manager manager;
    rb_host_api host = {RB_MODULE_ABI_VERSION, rb_log};
    rb_display screen = {NULL, NULL, emit_row, NULL};
    rb_ui_action page = RB_UI_NONE;
    unsigned i, active;
    if (argc > 2 || strlen(events) > RB_SIM_MAX_EVENTS) {
        fputs("ERRO: quantidade de eventos excedida\n", stderr);
        return 2;
    }
    for (i = 0; events[i]; ++i) {
        if (decode(events[i]) == (rb_ui_button)99) {
            fputs("ERRO: evento desconhecido\n", stderr);
            return 2;
        }
    }
    rb_ui_init(&ui);
    rb_manager_init(&manager);
    if (rb_manager_add(&manager, &rb_demo_module) != RB_OK) return 3;
    for (i = 0; events[i]; ++i) {
        rb_ui_button button = decode(events[i]);
        if (page != RB_UI_NONE) {
            if (button == RB_UI_BACK) {
                page = RB_UI_NONE;
            } else if (button == RB_UI_TRASH) {
                page = RB_UI_NONE;
                (void)rb_ui_event(&ui, button);
            } else if (page == RB_UI_MODULES && button == RB_UI_SET) {
                if (rb_manager_is_active(&manager, 0))
                    (void)rb_manager_disable(&manager, 0);
                else
                    (void)rb_manager_enable(&manager, 0, &host);
            }
            continue;
        }
        {
            rb_ui_action action = rb_ui_event(&ui, button);
            if (action != RB_UI_NONE) page = action;
        }
    }
    active = (unsigned)rb_manager_is_active(&manager, 0);
    printf("STATE\t%u\t%u\t%d\t%u\n",
           ui.opened, ui.selected, (int)page, active);
    if (page == RB_UI_NONE) {
        rb_ui_render(&ui, &screen);
    } else {
        const char *label = "REBOBINA / HOST";
        switch (page) {
        case RB_UI_HOME:
            label = "INICIO / DIAGNOSTICO";
            break;
        case RB_UI_TOOLS:
            label = "FERRAMENTAS / FUTURO";
            break;
        case RB_UI_MODULES:
            label = "MODULOS / GERENCIADOR";
            break;
        case RB_UI_ABOUT:
            label = "SOBRE / REBOBINA";
            break;
        default:
            break;
        }
        emit_row(NULL, 0, label, 0);
        if (page == RB_UI_HOME) {
            emit_row(NULL, 1, "Modo: simulacao de interface", 0);
            emit_row(NULL, 2, "Canon EOS 5D: nao conectada", 0);
            emit_row(NULL, 3, "Firmware real: nao verificado", 0);
        } else if (page == RB_UI_TOOLS) {
            emit_row(NULL, 1, "Intervalometro: planejado", 0);
            emit_row(NULL, 2, "Assistente de foco: planejado", 0);
            emit_row(NULL, 3, "Sem acesso ao sensor", 0);
        } else if (page == RB_UI_MODULES) {
            emit_row(NULL, 1, active ? "Hello: ATIVO (simulacao)" : "Hello: INATIVO (simulacao)", 1);
            emit_row(NULL, 2, "SET alterna modulo de exemplo", 0);
            emit_row(NULL, 3, "Soup Engine: nao integrado", 0);
        } else if (page == RB_UI_ABOUT) {
            emit_row(NULL, 1, "Rebobina / prototipo host", 0);
            emit_row(NULL, 2, "API de modulos: v1", 0);
            emit_row(NULL, 3, "Sem boot e sem firmware instalavel", 0);
        }
        emit_row(NULL, 4, "VOLTAR retorna ao menu", 0);
    }
    rb_manager_stop_all(&manager);
    return 0;
}
