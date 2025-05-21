#ifndef VERSION_MANAGER_H
#define VERSION_MANAGER_H

// Definição das versões disponíveis
// Descomente apenas UMA versão por vez

// Versão 1.0 - Básica (main.cpp padrão)
#define VERSION_BASIC

// Versão 2.0 - Debug Avançado (pagina debug boa 15-54 20-05-2025.ino)
//#define VERSION_DEBUG

// Versão 3.0 - Configuração Completa (vaidarcerto.ino)
//#define VERSION_FULL

// Informações sobre as versões
#ifdef VERSION_BASIC
    #define VERSION_NAME "1.0 - Básica"
    #define VERSION_DESC "Implementação padrão com funcionalidades essenciais"
#endif

#ifdef VERSION_DEBUG
    #define VERSION_NAME "2.0 - Debug Avançado"
    #define VERSION_DESC "Adiciona página de depuração melhorada"
#endif

#ifdef VERSION_FULL
    #define VERSION_NAME "3.0 - Configuração Completa"
    #define VERSION_DESC "Sistema completo com múltiplas páginas de configuração"
#endif

#endif // VERSION_MANAGER_H