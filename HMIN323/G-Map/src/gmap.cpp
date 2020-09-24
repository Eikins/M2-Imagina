#include "gmap.hpp"
/*------------------------------------------------------------------------*/

// Return the application of the alpha_deg on dart
GMap::id_t GMap::alpha(degree_t degree, id_t dart) const
{
    return alphas.at(dart)[degree];
}

// Return the application of a composition of alphas on dart
GMap::id_t GMap::alpha(degreelist_t degrees, id_t dart) const
{
    for (auto degree : degrees)
    {
        dart = alpha(degree, dart);
    }
    return dart;
}


//  Test if dart is free for alpha_degree (if it is a fixed point) 
bool GMap::is_free(degree_t degree, id_t dart) const
{
    return alpha(degree, dart) == dart;
}

/*
    Test the validity of the structure. 
    Check that alpha_0 and alpha_1 are involutions with no fixed points.
*/
bool GMap::is_valid() const
{
    degreelist_t involutionTestList{ 2, 0, 2, 0 };
    for (auto kp : alphas)
    {
        if (is_free(0, kp.first) || is_free(1, kp.first))
        {
            return false;
        }
        if (kp.first != alpha(involutionTestList, kp.first))
        {
            return false;
        }
    }
    return true;
}

/* 
    Create a new dart and return its id. 
    Set its alpha_i to itself (fixed points) 
*/
GMap::id_t GMap::add_dart()
{
    id_t dart = maxid++;
    alpha_container_t alpha_container(dart, dart, dart);
    alphas[dart] = alpha_container;
    return dart;
}

// Link the two darts with a relation alpha_degree if they are both free.
bool GMap::link_darts(degree_t degree, id_t dart1, id_t dart2)
{
    if (is_free(degree, dart1) && is_free(degree, dart2))
    {
        alphas[dart1][degree] = dart2;
        alphas[dart2][degree] = dart1;
        return true;
    }
    return false;
}

/* 
    Return the orbit of dart using a list of alpha relation.
    Example of use : gmap.orbit(0,[0,1]).
*/
GMap::idlist_t GMap::orbit(degreelist_t alphas, id_t dart)
{
    // As degreelist is not const, store the size
    // The compiler won't be able to optimize
    size_t dartCount = this->alphas.size();
    idlist_t result;
    idset_t marked;
    idlist_t stack{ dart };
    // Reserve space to avoid realloc
    marked.reserve(dartCount);
    stack.reserve(dartCount);

    while (stack.empty() == false)
    {
        id_t d = stack.back();
        stack.pop_back();
        if (marked.find(d) == marked.end())
        {
            result.push_back(d);
            marked.insert(d);
            for (auto degree : alphas)
            {
                stack.push_back(alpha(degree, d));
            }
        }
    }

    return result;
}

/*
    Return the ordered orbit of dart using a list of alpha relations by applying
    repeatingly the alpha relations of the list to dart.
    Example of use. gmap.orderedorbit(0,[0,1]).
    Warning: No fixed point for the given alpha should be contained.
*/
GMap::idlist_t GMap::orderedorbit(degreelist_t list_of_alpha_value, id_t dart)
{
    idlist_t result;
    id_t currentDart = dart;
    unsigned char currentAlphaIndex = 0;
    size_t count = list_of_alpha_value.size();
    do
    {
        result.push_back(currentDart);
        degree_t degree = list_of_alpha_value[currentAlphaIndex];
        currentAlphaIndex = (currentAlphaIndex + 1) % count;
        currentDart = alpha(degree, currentDart);
    } while (currentDart != dart);

    return result;
}

/*
    Sew two elements of degree 'degree' that start at dart1 and dart2.
    Determine first the orbits of dart to sew and heck if they are compatible.
    Sew pairs of corresponding darts
    # and if they have different embedding  positions, merge them. 
*/
bool GMap::sew_dart(degree_t degree, id_t dart1, id_t dart2)
{
    if (degree == 1)
    {
        link_darts(1, dart1, dart2);
    }
    else
    {
        if (degree == 0)
        {
            // TODO
        }
    }
}

// Compute the Euler-Poincare characteristic of the subdivision
int GMap::eulercharacteristic()
{
    return elements(0).size() - elements(1).size() + elements(2).size();
}


/*------------------------------------------------------------------------*/

/*
    Check if a dart of the orbit representing the vertex has already been 
    associated with a value in propertydict. If yes, return this dart, else
    return the dart passed as argument.
*/
//template<class T>
//GMap::id_t EmbeddedGMap<T>::get_embedding_dart(id_t dart) 
//{
//}
//
//
//
///*------------------------------------------------------------------------*/
//
//GMap3D GMap3D::dual()
//{
//}
//

/*------------------------------------------------------------------------*/
