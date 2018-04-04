/*
    This file is part of dfemtoolz software package.
*
    dfemtoolz software package is free software: 
*   you can redistribute it and/or modify it under the terms of the 
    GNU General Public License as published by the Free Software Foundation, 
*   either version 3 of the License, or (at your option) any later version.

*   dfemtoolz software package is distributed in the hope that 
    it will be useful, but WITHOUT ANY WARRANTY; without even the 
*   implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
    See the GNU General Public License for more details.
*
    You should have received a copy of the GNU General Public License
*   along with dfemtoolz software package.  
    If not, see <http://www.gnu.org/licenses/>.
*
    For any additional info contact author of this software:
*   
    Danko Milasinovic
*   dmilashinovic@gmail.com
    dmilashinovic@kg.ac.rs    
*/


#include "dnolimit.h"

int main ()
{
    Info * info = Info::createInfo();
    info->print_software_header("nolimit", 2017, 180313);
    info->print_info_message("");

    nolimit_Parameters * params = nolimit_Parameters::create_nolimit_Parameters();
    params->read_file();

    string dat_fileName = "input/pakf.dat";

    Timer * timer = Timer::createTimer();
    timer->set_start_time();

    Collection <Mesh_Node> nodez;
    Collection <Geom_Element>  elements;

    /// in this version - only sli is supported
    {
        SLI_Reader * sli_reader = SLI_Reader::create_SLI_Reader();
        sli_reader->read_file(nodez, elements, "input/simple_list.sli");
    }

    if (params->chek_all_elements)
    {
        bool all_elements_are_ok = true;

        for (UINT i = 1; i <= elements.get_size(); i++)
        {
            if (!this_brick_is_fine(elements[i], nodez))
            {
                info->print_info_message("element number " + utos(i) + " has det less than zero");
                all_elements_are_ok = false;
            }
        }

        if (all_elements_are_ok)
            info->print_info_message("all elements are fine");
    }

    if (params->calculate_volume_of_the_model)
    {
        double volume = 0.;

        for (UINT i = 1; i <= elements.get_size(); i++)
            volume += get_brick_volume(elements[i], nodez);

        info->print_info_message("model volume is " + dtos(volume));
    }

    UINT no_of_nodes = nodez.get_size();

    if (params->clear_previous_boundary)
    for (UINT i = 1; i <= nodez.get_size(); i++)
        nodez[i].clear_boundary();

    if ((params->scale_facX != 1.0) || (params->scale_facY != 1.0) || (params->scale_facZ != 1.0))
        scale_nodes(params->scale_facX, params->scale_facY, params->scale_facZ, nodez);


    if (params->calculate_surface)
    {
        info->print_info_message("finding surface...");

        {
            Collection <Geom_Element> quads;

            for (UINT i = 1; i <= elements.get_size(); i++)
                neighbours::create_facetz_add_facet_cogz_2_brick(elements[i], quads, nodez);

            for (UINT i = 1; i <= quads.get_size(); i++)
            if (nodez[quads[i].get_node(5)].get_flag())
                boundary_and_loads::set_velocity_boundary_marker_2_all_nodes_in_element(quads[i], nodez);
        }
    }


    timer->print_time_interval_in_seconds_from_start_to_now();

    info->print_info_message("writing files");

    if (params->print_pos_nodes || params->print_pos_elements)
    {
        POS_Printer * pos_printer = POS_Printer::create_POS_Printer();

        if (params->print_pos_nodes)
        {
            pos_printer->set_how_many_nodes_2_print(no_of_nodes);
            pos_printer->print_nodes_using_node_position_in_collection_as_ID_to_pos_file(nodez, "output/nodes_ID.pos");
            pos_printer->print_nodes_boundary_to_pos_file(nodez, "output/nodes_boundary.pos");
        }

        if (params->print_pos_elements)
            pos_printer->print_elems_to_pos_file(elements, constants::POS_BRICK, nodez, "output/elements.pos");
    }

    SLI_Printer * sli_printer = SLI_Printer::create_SLI_Printer();
    sli_printer->set_how_many_nodes_2_print(no_of_nodes);
    sli_printer->create_sli_file("output/simple_list.sli");
    sli_printer->first_block(no_of_nodes);
    sli_printer->print_nodes_using_node_position_in_collection_as_ID_into_sli(nodez);

    if (elements[1].how_many_nodes_per_element() == constants::BRICK)
    {
        sli_printer->second_block(elements.get_size(), constants::BRICK);
        sli_printer->print_elems_using_elem_position_in_collection_as_ID_into_sli(elements, nodez);
        sli_printer->third_block();
    }

    info->print_info_message("----");
    info->print_info_message("Number of nodez in final mesh:       " +
    utos(no_of_nodes));

    timer->print_time_interval_in_seconds_from_start_to_now();

    info->print_software_footer();
    press_any_key_2_continue();

    return 0;
}

