#include <assert.h>
#include <stddef.h>

#include "heatsim.h"
#include "log.h"

int heatsim_init(heatsim_t* heatsim, unsigned int dim_x, unsigned int dim_y) {
    /*
     * TODO: Initialiser tous les membres de la structure `heatsim`.
     *       Le communicateur doit être périodique. Le communicateur
     *       cartésien est périodique en X et Y.
     */

    /*
    MPI_Datatype HeatsimType;

    MPI_Datatype HeatsimFieldTypes[8] = 
    { MPI_Comm, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT };

    int HeatsimFieldLength[8] = { 1, 1, 1, 1, 1, 1, 1, 2 };

    MPI_Aint HeatsimFieldPosition[8] = { 
        offsetof(heatsim_t, communicator), offsetof(heatsim_t, rank_count), offsetof(heatsim_t, rank), 
        offsetof(heatsim_t, rank_north_peer), offsetof(heatsim_t, rank_south_peer), offsetof(heatsim_t, rank_east_peer),
        offsetof(heatsim_t, rank_west_peer), offsetof(heatsim_t, coordinates)
    };

    MPI_Type_create_struct(8, HeatsimFieldLength, HeatsimFieldPosition, HeatsimFieldTypes, &HeatsimType);
    MPI_Type_commit(&HeatsimType);
    */

    // heatsim->communicator = 
    int dims[2] = {dim_x, dim_y};
    int periods[2] = {1, 1};
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 0, &heatsim->communicator);

    // int rank; 
    MPI_Comm_rank(MPI_COMM_WORLD, &heatsim->rank);

    // int rank_count;
    MPI_Comm_size(MPI_COMM_WORLD, &heatsim->rank_count);

    // int coordinates[2];
    MPI_Cart_coords(heatsim->communicator, heatsim->rank, 2, heatsim->coordinates);

    // int rank_north_peer;
    int coords[2] = {heatsim->coordinates[0], heatsim->coordinates[1]-1};
    MPI_Cart_rank(heatsim->communicator, coords, &heatsim->rank_north_peer);

    // TODO : inversion nord/sud
    // https://www.codingame.com/playgrounds/47058/have-fun-with-mpi-in-c/mpi-process-topologies

    // int rank_south_peer;
    coords[1] += 2;
    MPI_Cart_rank(heatsim->communicator, coords, &heatsim->rank_south_peer);
    
    // int rank_east_peer;
    coords[1] -= 1;
    coords[0] += 1;
    MPI_Cart_rank(heatsim->communicator, coords, &heatsim->rank_east_peer);
    
    // int rank_west_peer;
    coords[0] -= 2;
    MPI_Cart_rank(heatsim->communicator, coords, &heatsim->rank_west_peer);
    
    return 1;
}

typedef struct grid_parameters {
    unsigned int width;
    unsigned int height;
    unsigned int padding;
} grid_param_t;

int heatsim_send_grids(heatsim_t* heatsim, cart2d_t* cart) {
    /*
     * TODO: Envoyer toutes les `grid` aux autres rangs. Cette fonction
     *       est appelé pour le rang 0. Par exemple, si le rang 3 est à la
     *       coordonnée cartésienne (0, 2), alors on y envoit le `grid`
     *       à la position (0, 2) dans `cart`.
     *
     *       Il est recommandé d'envoyer les paramètres `width`, `height`
     *       et `padding` avant les données. De cette manière, le receveur
     *       peut allouer la structure avec `grid_create` directement.
     *
     *       Utilisez `cart2d_get_grid` pour obtenir la `grid` à une coordonnée.
     */

    // Création de gridParamType
    MPI_Datatype gridParamType;
    MPI_Datatype paramFieldTypes[3] = {
        MPI_UNSIGNED, MPI_UNSIGNED, MPI_UNSIGNED
    };
    int paramFieldLength[3] = {1, 1, 1};
    MPI_Aint paramFieldPosition[3] = { 
        offsetof(grid_param_t, width), offsetof(grid_param_t, height), offsetof(grid_param_t, padding) 
    };
    MPI_Type_create_struct(3, paramFieldLength, paramFieldPosition, paramFieldTypes, &gridParamType);
    MPI_Type_commit(&gridParamType);

    // Création de gridDataType
    MPI_Datatype gridDataType;
    MPI_Datatype dataFieldTypes[1] = { MPI_DOUBLE };
    MPI_Aint dataFieldPosition[1] = { 0 };

    int coords[2];
    grid_t* grid;
    grid_param_t grid_param;
    MPI_Request request;
    MPI_Status status;

    // if (heatsim->rank_count == 1){
    //     printf("HERE");
    }

    for(int rank = 1; rank < heatsim->rank_count; rank++) {
        // Envoie de la requete (width, height, padding)
        MPI_Cart_coords(heatsim->communicator, rank, 2, coords);
        grid = cart2d_get_grid(cart, coords[0], coords[1]);
        grid_param.width = grid->width;
        grid_param.height = grid->height;
        grid_param.padding = grid->padding;
        MPI_Isend(&grid_param, 1, gridParamType, rank, 0, heatsim->communicator, &request);
        MPI_Wait(&request, &status);

        // Envoie de la requete (data)
        int dataFieldLength[1] = { grid->width * grid->height };
        MPI_Type_create_struct(1, dataFieldLength, dataFieldPosition, dataFieldTypes, &gridDataType);
        MPI_Type_commit(&gridDataType);
        MPI_Isend(grid->data, 1, gridDataType, rank, 0, heatsim->communicator, &request);
        MPI_Wait(&request, &status);
        // TODO si ca marche pas : rajouter MPI_Wait
    }
    
    return 1;
}

grid_t* heatsim_receive_grid(heatsim_t* heatsim) {
    /*
     * TODO: Recevoir un `grid ` du rang 0. Il est important de noté que
     *       toutes les `grid` ne sont pas nécessairement de la même
     *       dimension (habituellement ±1 en largeur et hauteur). Utilisez
     *       la fonction `grid_create` pour allouer un `grid`.
     *
     *       Utilisez `grid_create` pour allouer le `grid` à retourner.
     */
    
    // Création de gridParamType
    MPI_Datatype gridParamType;
    MPI_Datatype paramFieldTypes[3] = {
        MPI_UNSIGNED, MPI_UNSIGNED, MPI_UNSIGNED
    };
    int paramFieldLength[3] = {1, 1, 1};
    MPI_Aint paramFieldPosition[3] = { 
        offsetof(grid_param_t, width), offsetof(grid_param_t, height), offsetof(grid_param_t, padding) 
    };
    MPI_Type_create_struct(3, paramFieldLength, paramFieldPosition, paramFieldTypes, &gridParamType);
    MPI_Type_commit(&gridParamType);

    // Reception de la première requête
    MPI_Request request; 
    MPI_Status status;
    grid_param_t grid_param;
    MPI_Irecv(&grid_param, 1, gridParamType, 0, 0, heatsim->communicator, &request);
    MPI_Wait(&request, &status);
    
    // Création de gridDataType
    MPI_Datatype gridDataType;
    MPI_Datatype dataFieldTypes[1] = { MPI_DOUBLE };
    MPI_Aint dataFieldPosition[1] = { 0 };
    int dataFieldLength[1] = { grid_param.width * grid_param.height };
    MPI_Type_create_struct(1, dataFieldLength, dataFieldPosition, dataFieldTypes, &gridDataType);
    MPI_Type_commit(&gridDataType);

    grid_t* grid = grid_create(grid_param.width, grid_param.height, grid_param.padding);
    
    // Réception des données de la grille
    MPI_Irecv(grid->data, 1, gridDataType, 0, 0, heatsim->communicator, &request);
    MPI_Wait(&request, &status);

    return grid;
}

int heatsim_exchange_borders(heatsim_t* heatsim, grid_t* grid) {
    assert(grid->padding == 1);
    
    /*
     * TODO: Échange les bordures de `grid`, excluant le rembourrage, dans le
     *       rembourrage du voisin de ce rang. Par exemple, soit la `grid`
     *       4x4 suivante,
     *
     *                            +-------------+
     *                            | x x x x x x |
     *                            | x A B C D x |
     *                            | x E F G H x |
     *                            | x I J K L x |
     *                            | x M N O P x |
     *                            | x x x x x x |
     *                            +-------------+
     *
     *       où `x` est le rembourrage (padding = 1). Ce rang devrait envoyer
     *
     *        - la bordure [A B C D] au rang nord,
     *        - la bordure [M N O P] au rang sud,
     *        - la bordure [A E I M] au rang ouest et
     *        - la bordure [D H L P] au rang est.
     *
     *       Ce rang devrait aussi recevoir dans son rembourrage
     *
     *        - la bordure [A B C D] du rang sud,
     *        - la bordure [M N O P] du rang nord,
     *        - la bordure [A E I M] du rang est et
     *        - la bordure [D H L P] du rang ouest.
     *
     *       Après l'échange, le `grid` devrait avoir ces données dans son
     *       rembourrage provenant des voisins:
     *
     *                            +-------------+
     *                            | x m n o p x |
     *                            | d A B C D a |
     *                            | h E F G H e |
     *                            | l I J K L i |
     *                            | p M N O P m |
     *                            | x a b c d x |
     *                            +-------------+
     *
     *       Utilisez `grid_get_cell` pour obtenir un pointeur vers une cellule.
     */
    // Requête pour nord et sud de type MPI_DOUBLE
    // Requête pour est et ouest de type MPI_Type_vector
    // L’échange des bordures doit être effectué avec MPI_Send et MPI_Recv.

    // Obtenir la valeur : double* grid_get_cell(grid_t* grid, int i, int j) {
    double* gridCell;

    double* gridCellDst; 
    double* gridCellSrc;

    int rank;
    
    int coords[2];
    MPI_Cart_coords(heatsim->communicator, heatsim->rank, 2, coords);
    // printf("%d, %d, %d\n", coords[0], coords[1], heatsim->rank);
    // printf("%d\n", heatsim->rank_count);

    // if heatsim->rank == 0:
    // bloquant, les autres attendent le résultat de 0
    if (heatsim->rank_count == 1){
        for (int i = 0; i < grid->width; i++){
            // Echange Nord 
            gridCellDst = grid_get_cell(grid, i, -1);
            gridCellSrc = grid_get_cell(grid, i, grid->height-1);
            // memcpy(gridCellDst, gridCellSrc, 8); 
            // Copie le double mais fait un shallow copy
            // Affectation de double fait un deep copy
            *gridCellSrc = *gridCellDst;

            // Echange Sud 
            gridCellDst = grid_get_cell(grid, i, grid->height);
            gridCellSrc = grid_get_cell(grid, i, 0);
            *gridCellSrc = *gridCellDst;
        }

        for (int i = 0; i < grid->height; i++){
            // Echange Est 
            gridCellDst = grid_get_cell(grid, grid->width, i);
            gridCellSrc = grid_get_cell(grid, 0, i);
            *gridCellSrc = *gridCellDst;

            // Echange Ouest 
            gridCellDst = grid_get_cell(grid, -1, i);
            gridCellSrc = grid_get_cell(grid, grid->width-1, i);
            *gridCellSrc = *gridCellDst;
        }

        // printf("DAVID : Exchange finished (rank = 0)\n");
        return 1;
    }

    // TODO if rank == 0 ??


    // printf("PADDING %d %d \n", grid->width, grid->width_padded);
    // Échange de la bordure nord, on envoie la bordure nord au nord
    rank = heatsim->rank_north_peer;
    for(int i = 0; i < grid->width; i++) {
        gridCell = grid_get_cell(grid, i, 0);
        MPI_Send(gridCell, 1, MPI_DOUBLE, rank, i, heatsim->communicator);
    }   

    
    // Échange de la bordure sud
    rank = heatsim->rank_south_peer;
    for(int i = 0; i < grid->width; i++) {
        gridCell = grid_get_cell(grid, i, grid->height-1);
        MPI_Send(gridCell, 1, MPI_DOUBLE, rank, i+ grid->width, heatsim->communicator);
    }

    
    double cellVector[grid->height];
    MPI_Datatype doubleVector;
    MPI_Type_vector(grid->height, 1, 1, MPI_DOUBLE, &doubleVector);
    MPI_Type_commit(&doubleVector);

    // Échange de la bordure Ouest
    rank = heatsim->rank_west_peer;
    for (int j = 0; j < grid->height; j++) {
        cellVector[j] = *grid_get_cell(grid, 0, j);
    }
    MPI_Send(cellVector, 1, doubleVector, rank, 1, heatsim->communicator);

    // Échange de la bordure Est
    rank = heatsim->rank_east_peer;
    for (int j = 0; j < grid->height; j++) {
        cellVector[j] = *grid_get_cell(grid, grid->width-1, j);
    }
    MPI_Send(cellVector, 1, doubleVector, rank, 2, heatsim->communicator);

    MPI_Status status;
    // Réception bordure nord
    rank = heatsim->rank_north_peer;
    for (int i = 0; i < grid->width; i++) {
        // gridCell = grid_get_cell_padded(grid, i, 0);
        gridCellDst = grid_get_cell(grid, i, -1);
        MPI_Recv(gridCellDst, 1, MPI_DOUBLE, rank, i+grid->width, heatsim->communicator, &status);
   
    } 
    
    // Réception bordure sud
    rank = heatsim->rank_south_peer;
    for (int i = 0; i < grid->width; i++) {
        // gridCell = grid_get_cell_padded(grid, i, grid->height);
        gridCellDst = grid_get_cell(grid, i, grid->height);
        MPI_Recv(gridCellDst, 1, MPI_DOUBLE, rank, i, heatsim->communicator, &status);
    } 

    // Réception bordure ouest
    rank = heatsim->rank_west_peer;
    MPI_Recv(cellVector, 1, doubleVector, rank, 2, heatsim->communicator, &status);
    for (int j = 0; j < grid->height; j++) {
        // gridCell = grid_get_cell_padded(grid, 0, j);
        gridCellDst = grid_get_cell(grid, -1, j);
        *gridCellDst = cellVector[j];
    }

    // // Réception bordure est
    rank = heatsim->rank_east_peer;
    MPI_Recv(cellVector, 1, doubleVector, rank, 1, heatsim->communicator, &status);
    for (int j = 0; j < grid->height; j++) {
        // gridCell = grid_get_cell_padded(grid, grid->height, 0);
        gridCellDst = grid_get_cell(grid, grid->width, j);
        *gridCellDst = cellVector[j];
    }

    return 1;
}

int heatsim_send_result(heatsim_t* heatsim, grid_t* grid) {
    assert(grid->padding == 0);

    if (heatsim->rank == 0){
        return -1;
    }
    printf("DAVID : %d enters in heatsim_send_result\n", heatsim->rank);

    // L’envoie et la réception de la grille finale doit être effectué avec MPI_Send et MPI_Recv.
    // Les données (data) de la grille doit être de type définit avec MPI_Type_struct.
    /*
     * TODO: Envoyer les données (`data`) du `grid` résultant au rang 0. Le
     *       `grid` n'a aucun rembourage (padding = 0);
     */
    MPI_Datatype gridDataType;
    MPI_Datatype dataFieldTypes[1] = { MPI_DOUBLE };
    MPI_Aint dataFieldPosition[1] = { 0 };
    int dataFieldLength[1] = { grid->width * grid->height };
    MPI_Type_create_struct(1, dataFieldLength, dataFieldPosition, dataFieldTypes, &gridDataType);
    MPI_Type_commit(&gridDataType);

    // TAG 10 
    MPI_Send(grid->data, 1, gridDataType, 0, 10, heatsim->communicator);
    // printf("DAVID : %d out of heatsim_send_results\n", heatsim->rank);
    return 1;
}

int heatsim_receive_results(heatsim_t* heatsim, cart2d_t* cart) {
    /*
     * TODO: Recevoir toutes les `grid` des autres rangs. Aucune `grid`
     *       n'a de rembourage (padding = 0).
     *
     *       Utilisez `cart2d_get_grid` pour obtenir la `grid` à une coordonnée
     *       qui va recevoir le contenue (`data`) d'un autre noeud.
     */
    
    // Création de gridDataType
    printf("DAVID : 0 entre dans heatsim_receive_results\n");
    MPI_Datatype gridDataType;


    MPI_Datatype dataFieldTypes[1] = { MPI_DOUBLE };
    MPI_Aint dataFieldPosition[1] = { 0 };


    int coords[2];
    grid_t* grid;
    MPI_Status status;

    for(int rank = 1; rank < heatsim->rank_count; rank++) {
        // Get grid 
        MPI_Cart_coords(heatsim->communicator, rank, 2, coords);
        grid = cart2d_get_grid(cart, coords[0], coords[1]);

        // Reception de la requete (data)
        int dataFieldLength[1] = { grid->width * grid->height };
        MPI_Type_create_struct(1, dataFieldLength, dataFieldPosition, dataFieldTypes, &gridDataType);
        MPI_Type_commit(&gridDataType);
        // printf("DAVID : 0 is waiting %d message with %d\n", rank, grid->width*grid->height);
        // TAG 10 
        MPI_Recv(grid->data, 1, gridDataType, rank, 10, heatsim->communicator, &status);
        // printf("DAVID : %d RECEIVED\n", rank);
    }

    // printf("DAVID : 0 out of heatsim_receive_results\n");
    return 1;
}
