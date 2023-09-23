#include <SFML/Window/Keyboard.hpp>
#include <ios>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <tuple>
#include <chrono>
#include "cartesian_grid_of_speed.hpp"
#include "vortex.hpp"
#include "cloud_of_points.hpp"
#include "runge_kutta.hpp"
#include "screen.hpp"
#include <mpi.h>

#define FLAG_COMMAND 11
#define FLAG_CLOUD 12
#define FLAG_GRID 13
#define FLAG_VORTICE 14

// prof's function to read config
auto readConfigFile(std::ifstream &input)
{
    using point = Simulation::Vortices::point;

    int isMobile;
    std::size_t nbVortices;
    Numeric::CartesianGridOfSpeed cartesianGrid;
    Geometry::CloudOfPoints cloudOfPoints;
    constexpr std::size_t maxBuffer = 8192;
    char buffer[maxBuffer];
    std::string sbuffer;
    std::stringstream ibuffer;
    // Lit la première ligne de commentaire :
    input.getline(buffer, maxBuffer); // Relit un commentaire
    input.getline(buffer, maxBuffer); // Lecture de la grille cartésienne
    sbuffer = std::string(buffer, maxBuffer);
    ibuffer = std::stringstream(sbuffer);
    double xleft, ybot, h;
    std::size_t nx, ny;
    ibuffer >> xleft >> ybot >> nx >> ny >> h;
    cartesianGrid = Numeric::CartesianGridOfSpeed({nx, ny}, point{xleft, ybot}, h);
    input.getline(buffer, maxBuffer); // Relit un commentaire
    input.getline(buffer, maxBuffer); // Lit mode de génération des particules
    sbuffer = std::string(buffer, maxBuffer);
    ibuffer = std::stringstream(sbuffer);
    int modeGeneration;
    ibuffer >> modeGeneration;
    if (modeGeneration == 0) // Génération sur toute la grille
    {
        std::size_t nbPoints;
        ibuffer >> nbPoints;
        cloudOfPoints = Geometry::generatePointsIn(nbPoints, {cartesianGrid.getLeftBottomVertex(), cartesianGrid.getRightTopVertex()});
    }
    else
    {
        std::size_t nbPoints;
        double xl, xr, yb, yt;
        ibuffer >> xl >> yb >> xr >> yt >> nbPoints;
        cloudOfPoints = Geometry::generatePointsIn(nbPoints, {point{xl, yb}, point{xr, yt}});
    }
    // Lit le nombre de vortex :
    input.getline(buffer, maxBuffer); // Relit un commentaire
    input.getline(buffer, maxBuffer); // Lit le nombre de vortex
    sbuffer = std::string(buffer, maxBuffer);
    ibuffer = std::stringstream(sbuffer);
    try
    {
        ibuffer >> nbVortices;
    }
    catch (std::ios_base::failure &err)
    {
        std::cout << "Error " << err.what() << " found" << std::endl;
        std::cout << "Read line : " << sbuffer << std::endl;
        throw err;
    }
    Simulation::Vortices vortices(nbVortices, {cartesianGrid.getLeftBottomVertex(),
                                               cartesianGrid.getRightTopVertex()});
    input.getline(buffer, maxBuffer); // Relit un commentaire
    for (std::size_t iVortex = 0; iVortex < nbVortices; ++iVortex)
    {
        input.getline(buffer, maxBuffer);
        double x, y, force;
        std::string sbuffer(buffer, maxBuffer);
        std::stringstream ibuffer(sbuffer);
        ibuffer >> x >> y >> force;
        vortices.setVortex(iVortex, point{x, y}, force);
    }
    input.getline(buffer, maxBuffer); // Relit un commentaire
    input.getline(buffer, maxBuffer); // Lit le mode de déplacement des vortex
    sbuffer = std::string(buffer, maxBuffer);
    ibuffer = std::stringstream(sbuffer);
    ibuffer >> isMobile;
    return std::make_tuple(vortices, isMobile, cartesianGrid, cloudOfPoints);
}

int main(int argc, char *argv[])
{
    int size, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    char const *filename;
    if (argc == 1)
    {
        std::cout << "Usage : vortexsimulator <nom fichier configuration>" << std::endl;
        return EXIT_FAILURE;
    }

    filename = argv[1];
    std::ifstream fich(filename);
    auto config = readConfigFile(fich);
    fich.close();

    std::size_t resx = 800, resy = 600;
    if (argc > 3)
    {
        resx = std::stoull(argv[2]);
        resy = std::stoull(argv[3]);
    }

    auto vortices = std::get<0>(config);
    auto isMobile = std::get<1>(config);
    auto grid = std::get<2>(config);
    auto cloud = std::get<3>(config);

    grid.updateVelocityField(vortices);

    bool animate = true;
    bool nextFrame = false;

    double dt = 0.1;

    // processus afichage
    if (rank == 0)
    {
        Graphisme::Screen myScreen({resx, resy}, {grid.getLeftBottomVertex(), grid.getRightTopVertex()});
        std::cout << "######## Vortex simultor ########" << std::endl << std::endl;
        std::cout << "Press P for play animation " << std::endl;
        std::cout << "Press S to stop animation" << std::endl;
        std::cout << "Press E to end program" << std::endl;
        std::cout << "Press right cursor to advance step by step in time" << std::endl;
        std::cout << "Press down cursor to halve the time step" << std::endl;
        std::cout << "Press up cursor to double the time step" << std::endl;

        // variables pour FPS
        auto start = std::chrono::system_clock::now();
        int frames = 0;
        int fps = 0;

        // variable qui prends les inputs du clavier
        char key;
        while (myScreen.isOpen())
        {
            key = '*';
            // on inspecte tous les évènements de la fenêtre qui ont été émis depuis la précédente itération
            sf::Event event;
            while (myScreen.pollEvent(event))
            {
                if (event.type == sf::Event::Resized)
                    myScreen.resize(event);
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::P)){
                    key = 'P';
                    animate = true;
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)){
                    key = 'S';
                    animate = false;
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
                {
                    key = 'U';
                    dt *= 2;
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
                {
                    key = 'D';
                    dt /= 2;
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
                {
                    key = 'R';
                    nextFrame = true;
                }
                if (event.type == sf::Event::Closed)
                    key = 'E';
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
                    key = 'E';
                // si une clée a été apuié, envoie l'evenment a touts processus
                if (key != '*'){
                    for(int i = 1; i < size; i++){
                        MPI_Send(&key, 1, MPI_CHAR, i, FLAG_COMMAND, MPI_COMM_WORLD);
                    }
                    if(key == 'E'){
                        myScreen.close();
                        MPI_Finalize();
                        return 0;
                    }
                }
            }
            if (animate || nextFrame){
                key = 'N';
                MPI_Send(&key, 1, MPI_CHAR, 1, FLAG_COMMAND, MPI_COMM_WORLD);

                // reçoit les donnés calculés par le processus 1
                MPI_Recv(cloud.data(), 2 * cloud.numberOfPoints(), MPI_DOUBLE, 1, FLAG_CLOUD, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
                if(isMobile){
                    MPI_Recv(grid.data(), 2 * grid.numberOfPoints(), MPI_DOUBLE, 1, FLAG_GRID, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
                    MPI_Recv(vortices.data(), 3 * vortices.numberOfVortices(), MPI_DOUBLE, 1, FLAG_VORTICE, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
                }
                nextFrame = false;
                frames++;
            }

            // mis a jour écran
            myScreen.clear(sf::Color::Black);

            std::string strDt = std::string("Time step : ") + std::to_string(dt);
            myScreen.drawText(strDt, Geometry::Point<double>{50, double(myScreen.getGeometry().second - 96)});
            auto start_af = std::chrono::system_clock::now();
            
            myScreen.displayVelocityField(grid, vortices);
            auto end_af = std::chrono::system_clock::now();
            std::chrono::duration<double> diff_af = end_af - start_af;
            
            myScreen.displayParticles(grid, vortices, cloud);

            auto end = std::chrono::system_clock::now();
            std::chrono::duration<double> diff = end - start;
            if (diff.count() >= 1.0)
            {
                fps = frames;
                start = end;
                frames = 0;
            }
            std::string str_fps = std::string("FPS : ") + std::to_string(fps);
            myScreen.drawText(str_fps, Geometry::Point<double>{300, double(myScreen.getGeometry().second - 96)});
            myScreen.display();
            
        }
    }

    // processus de calcul
    if (rank == 1)
    {
        while (true)
        {
            bool advance = false;
            bool calculate = false;
            char key = '*';
            
            // reçoit commande
            MPI_Recv(&key, 1, MPI_CHAR, 0, FLAG_COMMAND, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
            switch (key)
            {
            case 'E':
                MPI_Finalize();
                return 0;
            case 'P':
                animate = true;
                break;
            case 'S':
                animate = false;
                break;
            case 'U':
                dt *= 2;
                break;
            case 'D':
                dt /= 2;
                break;
            case 'R':
                advance = true;
                calculate = true;
                break;
            case 'N':
                calculate = true;
                break;
            default:
                break;
            }
            
            if(calculate){
                if (animate | advance)
                {
                    cloud = Numeric::solve_RK4_fixed_vortices(dt, grid, cloud); 
                    if (isMobile)
                        Numeric::updateVortices(dt, grid, vortices, cloud);
                    
                    MPI_Send(cloud.data(), 2 * cloud.numberOfPoints(), MPI_DOUBLE, 0, FLAG_CLOUD, MPI_COMM_WORLD);
                    
                    if(isMobile){
                        MPI_Send(grid.data(), 2 * grid.numberOfPoints(), MPI_DOUBLE, 0, FLAG_GRID, MPI_COMM_WORLD);
                        MPI_Send(vortices.data(), 3 * vortices.numberOfVortices(), MPI_DOUBLE, 0, FLAG_VORTICE, MPI_COMM_WORLD);
                    }
                }
            }
            
        }
    }

    MPI_Finalize();
    return 0;
}
