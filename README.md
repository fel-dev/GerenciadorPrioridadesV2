# GerenciadorPrioridadesV2

Aplicativo Windows para gerenciar prioridades de processos.
📁 GerenciadorPrioridadesV2/
├── main.cpp                     ← ponto de entrada (WinMain)
├── gui.cpp / gui.h             ← criação da janela e controles visuais
├── processos.cpp / processos.h ← funções de leitura e alteração de processos
├── eventos.cpp / eventos.h     ← onde tratamos os comandos dos botões
├── util.cpp / util.h           ← parsing, strings, helpers auxiliares
└── GerenciadorPrioridadesV2.vcxproj
> Cada arquivo/função tem responsabilidade única, facilitando manutenção e expansão.