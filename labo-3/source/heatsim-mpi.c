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
    MPI_Cart_coords(heatsim->communicator, heatsim->rank, heatsim->rank_count, heatsim->coordinates);

    // int rank_north_peer;
    int coords[2] = {heatsim->coordinates[0], heatsim->coordinates[1]+1};
    MPI_Cart_rank(heatsim->communicator, coords, &heatsim->rank_north_peer);

    // int rank_south_peer;
    coords[1] -= 2;
    MPI_Cart_rank(heatsim->communicator, coords, &heatsim->rank_south_peer);
    
    // int rank_east_peer;
    coords[1] += 1;
    coords[0] += 1;
    MPI_Cart_rank(heatsim->communicator, coords, &heatsim->rank_east_peer);
    
    // int rank_west_peer;
    coords[0] -= 2;
    MPI_Cart_rank(heatsim->communicator, coords, &heatsim->rank_west_peer);
    
    return 1;
fail_exit:
    return -1;
}

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

fail_exit:
    return -1;
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

fail_exit:
    return NULL;
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

fail_exit:
    return -1;
}

int heatsim_send_result(heatsim_t* heatsim, grid_t* grid) {
    assert(grid->padding == 0);

    /*
     * TODO: Envoyer les données (`data`) du `grid` résultant au rang 0. Le
     *       `grid` n'a aucun rembourage (padding = 0);
     */

fail_exit:
    return -1;
}

int heatsim_receive_results(heatsim_t* heatsim, cart2d_t* cart) {
    /*
     * TODO: Recevoir toutes les `grid` des autres rangs. Aucune `grid`
     *       n'a de rembourage (padding = 0).
     *
     *       Utilisez `cart2d_get_grid` pour obtenir la `grid` à une coordonnée
     *       qui va recevoir le contenue (`data`) d'un autre noeud.
     */

fail_exit:
    return -1;
}
