#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "servicos.h" // Inclui o cabeçalho que define estruturas (DadosSistema) e funções de serviço.

// --- Função Auxiliar ---

/**
 * @brief Limpa o buffer de entrada (stdin).
 * Essencial após o uso de scanf() ou antes de um fgets() para 
 * descartar quaisquer caracteres residuais, incluindo a quebra de linha ('\n').
 */
void limpar_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// --- Função Principal ---

int main() {
    DadosSistema sistema;           // Estrutura principal que armazena todos os dados (alunos, turmas).
    int opcao;                      // Variável para armazenar a opção escolhida no menu.
    int nivel_acesso = -1;          // -1 significa que o usuário ainda não está logado.
    
    // 1. Carrega dados persistentes (de arquivo) para a estrutura do sistema.
    carregar_dados(&sistema);

    // 2. Tenta logar o usuário antes de iniciar o loop principal.
    if (!realizar_login(&nivel_acesso)) {
        // Se a função realizar_login retornar 0 (falha), encerra o programa.
        printf("Falha no login ou usuario/senha invalidos. Encerrando o sistema.\n");
        return 1; 
    }

    // 3. Loop principal do menu (Continua até que a opção de 'Sair' seja escolhida)
    do {
        // 3.1. Exibe o cabeçalho do menu, identificando o nível de acesso do usuário.
        printf("\n--- MENU PRINCIPAL (%s) ---\n", 
               (nivel_acesso == NIVEL_ADMIN) ? "ADMINISTRADOR" : // Se for ADMIN
               (nivel_acesso == NIVEL_PROFESSOR) ? "PROFESSOR" : // Se for PROFESSOR
               "ALUNO");                                         // Caso contrário (ALUNO)
               
        // Exibe o status atual do sistema (quantas turmas/alunos cadastrados).
        printf("Turmas: %d / %d | Alunos: %d / %d\n", sistema.total_turmas, MAX_TURMAS, sistema.total_alunos, MAX_ALUNOS);
        printf("-----------------------------\n");
        
        // --- Opções Comuns a Professor e Admin (CRUD de dados) ---
        // As opções 1 a 3 só são exibidas se o nível de acesso for PROFESSOR ou superior (ADMIN).
        if (nivel_acesso >= NIVEL_PROFESSOR) {
            printf("1. Cadastrar Turma\n");
            printf("2. Cadastrar Aluno\n");
            printf("3. Lancar Notas e Calcular Media (PROF/ADMIN)\n");
        }

        // --- Opções Comuns a Todos ---
        printf("4. Gerar Relatorio de Turma (TODOS)\n"); 
        
        // --- Opções Exclusivas do Admin (Manutenção e CRUD Total) ---
        // As opções 5 a 8 só são exibidas se o nível de acesso for ADMINISTRADOR.
        if (nivel_acesso == NIVEL_ADMIN) {
            printf("5. Ordenar Alunos por Nome\n");
            printf("-----------------------------\n");
            printf("6. EDITAR Dados do Aluno\n");
            printf("7. EXCLUIR Aluno (Logico)\n");
            printf("8. EXCLUIR Turma (Logico + Cascata)\n");
        }
        
        printf("9. Sair\n"); 
        printf("Escolha uma opcao: ");

        // 3.2. Leitura e tratamento de buffer para a opção escolhida.
        if (scanf("%d", &opcao) != 1) {
            limpar_buffer();
            opcao = 0; // Se a entrada for inválida (não for um número), define como 0 (default/inválido).
        } else {
            limpar_buffer(); // Limpa o '\n' restante após o scanf bem-sucedido.
        }

        // --- 3.3. BLOQUEIO DE ACESSO (Guardrail) ---
        // Verifica se o usuário escolheu uma opção para a qual não tem permissão.
        if (
            (nivel_acesso < NIVEL_PROFESSOR && (opcao >= 1 && opcao <= 3)) || // Bloqueia CRUD (1-3) para ALUNO
            (nivel_acesso < NIVEL_ADMIN && (opcao >= 5 && opcao <= 8))       // Bloqueia ADMIN features (5-8) para PROF/ALUNO
        ) {
            if (opcao != 4 && opcao != 9) { // Permite 4 (Relatório) e 9 (Sair), mesmo que estejam no range.
                printf("ACESSO NEGADO: Esta opcao nao esta disponivel para seu nivel de usuario.\n");
                continue; // Pula o resto do loop e volta para o início do menu.
            }
        }
        
        // 3.4. Processamento da opção escolhida.
        switch (opcao) {
            case 1: { // Cadastrar Turma (PROF/ADMIN)
                char nome[TAM_NOME];
                int vagas;
                printf("Nome da Turma: ");
                // Lê o nome da turma de forma segura (fgets) e remove o '\n'.
                fgets(nome, TAM_NOME, stdin);
                nome[strcspn(nome, "\n")] = 0;
                printf("Vagas Maximas: ");
                scanf("%d", &vagas);
                limpar_buffer();
                
                if (adicionar_turma(&sistema, nome, vagas)) {
                    printf("SUCESSO: Turma '%s' cadastrada.\n", nome); 
                    salvar_dados(&sistema); // Salva as alterações no arquivo.
                } else {
                    printf("ERRO: Nao foi possivel cadastrar a turma (limite atingido ou erro interno).\n");
                }
                break;
            }
            case 2: { // Cadastrar Aluno (PROF/ADMIN)
                listar_todas_turmas(&sistema); // Ajuda o usuário a escolher o ID da turma.
                char nome[TAM_NOME], ra[TAM_RA];
                int id_turma;
                
                printf("Nome do Aluno: ");
                fgets(nome, TAM_NOME, stdin);
                nome[strcspn(nome, "\n")] = 0;
                
                printf("RA (max %d digitos): ", TAM_RA - 1);
                fgets(ra, TAM_RA, stdin);
                ra[strcspn(ra, "\n")] = 0;
                
                printf("ID da Turma: ");
                if (scanf("%d", &id_turma) != 1) { 
                    limpar_buffer(); 
                    printf("ERRO: ID de turma invalido.\n"); 
                    break; 
                }
                limpar_buffer();
                
                if (adicionar_aluno(&sistema, nome, ra, id_turma)) {
                    printf("SUCESSO: Aluno '%s' (RA: %s) adicionado a Turma ID %d.\n", nome, ra, id_turma);
                    salvar_dados(&sistema);
                }
                break;
            }
            case 3: { // Lançar Notas e Recalcular Média (PROF/ADMIN)
                char ra[TAM_RA];
                float n1, n2, n3;
                printf("RA do Aluno: ");
                fgets(ra, TAM_RA, stdin);
                ra[strcspn(ra, "\n")] = 0;
                
                // Leitura das notas com tratamento de erro imediato
                printf("Nota 1: ");
                if (scanf("%f", &n1) != 1) { limpar_buffer(); printf("Entrada invalida.\n"); break; }
                printf("Nota 2: ");
                if (scanf("%f", &n2) != 1) { limpar_buffer(); printf("Entrada invalida.\n"); break; }
                printf("Nota 3: ");
                if (scanf("%f", &n3) != 1) { limpar_buffer(); printf("Entrada invalida.\n"); break; }
                limpar_buffer();
                
                // Passa o nível de acesso para a função fazer a verificação interna (se necessário)
                if (lancar_notas_e_atualizar_media(&sistema, ra, n1, n2, n3, nivel_acesso)) {
                    salvar_dados(&sistema);
                }
                break;
            }
            case 4: { // Gerar Relatório de Turma (TODOS)
                int id_turma;
                listar_todas_turmas(&sistema);
                printf("ID da Turma para Relatorio: ");
                if (scanf("%d", &id_turma) != 1) { limpar_buffer(); printf("ERRO: ID de turma invalido.\n"); break; }
                limpar_buffer();
                gerar_relatorio_turma(&sistema, id_turma);
                break;
            }
            case 5: { // Ordenar Alunos por Nome (ADMIN)
                if (sistema.total_alunos == 0) { 
                    printf("AVISO: Nao ha alunos para ordenar.\n"); 
                    break; 
                }
                ordenar_alunos_por_nome(&sistema); // Chama a função de ordenação (ex: Quicksort, Bubble Sort).
                salvar_dados(&sistema); 
                printf("SUCESSO: Lista de alunos ordenada por nome e salva.\n");
                break;
            }
            case 6: { // EDITAR Dados do Aluno (ADMIN)
                listar_todas_turmas(&sistema);
                char ra_antigo[TAM_RA], nome_novo[TAM_NOME];
                int id_turma_nova = 0; // '0' é um valor de controle que significa 'manter turma'.
                
                printf("RA do Aluno para editar: ");
                fgets(ra_antigo, TAM_RA, stdin);
                ra_antigo[strcspn(ra_antigo, "\n")] = 0;
                
                printf("Novo Nome do Aluno (Deixe Vazio para manter o atual): ");
                fgets(nome_novo, TAM_NOME, stdin);
                nome_novo[strcspn(nome_novo, "\n")] = 0;

                printf("Novo ID da Turma (Digite 0 para manter a atual): ");
                if (scanf("%d", &id_turma_nova) != 1) { 
                    limpar_buffer(); 
                    id_turma_nova = 0; 
                }
                limpar_buffer();

                if (editar_dados_aluno(&sistema, ra_antigo, nome_novo, id_turma_nova)) {
                    salvar_dados(&sistema);
                }
                break;
            }
            case 7: { // EXCLUIR Aluno (Lógico) (ADMIN)
                char ra[TAM_RA];
                printf("RA do Aluno para exclusao: ");
                fgets(ra, TAM_RA, stdin);
                ra[strcspn(ra, "\n")] = 0;

                if (excluir_aluno_por_ra(&sistema, ra)) {
                    salvar_dados(&sistema);
                }
                break;
            }
            case 8: { // EXCLUIR Turma (Lógico com Exclusão em Cascata) (ADMIN)
                listar_todas_turmas(&sistema);
                int id;
                printf("ID da Turma para exclusao: ");
                if (scanf("%d", &id) != 1) { limpar_buffer(); printf("ERRO: ID de turma invalido.\n"); break; }
                limpar_buffer();

                if (excluir_turma_por_id(&sistema, id)) {
                    salvar_dados(&sistema); // Salva após a exclusão da turma e dos alunos relacionados (cascata).
                }
                break;
            }
            case 9: // Sair
                printf("Encerrando o Sistema Academico...\n");
                salvar_dados(&sistema); // Garante que a última versão dos dados seja salva.
                printf("Ate logo!\n");
                break;
            default:
                // Trata opções inválidas (e a opção '0' de entradas não numéricas).
                printf("Opcao invalida. Por favor, escolha uma opcao valida.\n");
                break;
        }
        
    } while (opcao != 9); // O loop continua enquanto a opção 9 (Sair) não for escolhida.

    return 0; // Retorno de sucesso.
}