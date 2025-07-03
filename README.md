# 🚀 Gerenciador de Prioridades — v2.2.0

Aplicativo leve e responsivo para Windows que permite visualizar, modificar e gerenciar a prioridade dos processos ativos. Feito em C++ com interface nativa WinAPI.

---

## 🖥️ Funcionalidades
- ✅ Interface gráfica moderna e intuitiva (ListView, emojis, layout adaptativo)
- 🧠 Alteração real de prioridade de processos (rebaixar, elevar, restaurar)
- ⭐ Sistema de favoritos e log de sessões
- ⏪ Botão "Reverter alterações" via menu de contexto
- ⌨️ Atalhos de teclado para navegação rápida
- 🌍 Internacionalização via recursos (PT-BR, preparado para EN)
- 📁 Modularidade: organização clara entre GUI, lógica, eventos e utilitários

---

## 📂 Estrutura do Projeto

Aplicativo Windows para gerenciar prioridades de processos.


    📁 GerenciadorPrioridadesV2/
    ├── main.cpp                     ← ponto de entrada (WinMain)
    ├── gui.cpp / gui.h             ← criação da janela e controles visuais
    ├── processos.cpp / processos.h ← funções de leitura e alteração de processos
    ├── eventos.cpp / eventos.h     ← onde tratamos os comandos dos botões
    ├── util.cpp / util.h           ← parsing, strings, helpers auxiliares
    └── GerenciadorPrioridadesV2.vcxproj

> Cada arquivo/função tem responsabilidade única, facilitando manutenção e expansão.

---

## 📈 Roadmap
- **v2.3**: Menu para alteração de tamanho de fonte / acessibilidade
- **v2.4**: Suporte a grupos/categorias de processos
- **v2.5+**: Monitoramento em tempo real, temas, integração com serviços
- **v3.0**: Interface redesenhada e recursos avançados com automações

---

## 🤝 Contribuições
Pull Requests são bem-vindas! Sugestões, issues e colaborações são muito apreciadas 😄

---

## 📜 Licença
Este projeto está sob a [MIT License](LICENSE)

