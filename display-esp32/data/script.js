$(document).ready(function(){
    $("#appliquer").click(function(){
        var valeur = $("#choixDelayLed").val();
        $.post("delayLed",{
            valeurDelayLed: valeur
        });
    });
});
// Valeur par défauts 
$(document).ready(function(){
    $.ajax( {
        type: "GET",
        url: "param.xml",
        dataType: "xml",
        success: function(xml) 
                 {
                   $(xml).find("param").each(
                     function()
                     {
                        $(this).find("beer_name").each(
                        function()
                        {    document.getElementById("beer_name").value = $(this).text();
                            $(this).value =document.getElementById(beer_name).value;
                      });   
                                          });

                }
    });
    
});

//BEER IBU
$(document).ready(function(){
    $.ajax( {
        type: "GET",
        url: "param.xml",
        dataType: "xml",
        success: function(xml) 
                 {
                   $(xml).find("param").each(
                     function()
                     {
                        $(this).find("beer_ibu").each(
                        function()
                        {    document.getElementById("beer_ibu").value = $(this).text();
                            $(this).value =document.getElementById(beer_ibu).value;
                      });   
                    });

                }
    });
    
});

//BEER SG
$(document).ready(function(){
    $.ajax( {
        type: "GET",
        url: "param.xml",
        dataType: "xml",
        success: function(xml) 
                 {
                   $(xml).find("param").each(
                     function()
                     {
                        $(this).find("beer_sg").each(
                        function()
                        {    document.getElementById("beer_sg").value = $(this).text();
                            $(this).value =document.getElementById(beer_sg).value;
                      });   
                    });

                }
    });
    
});

//BEER ABV
$(document).ready(function(){
    $.ajax( {
        type: "GET",
        url: "param.xml",
        dataType: "xml",
        success: function(xml) 
                 {
                   $(xml).find("param").each(
                     function()
                     {
                        $(this).find("beer_abv").each(
                        function()
                        {    document.getElementById("beer_abv").value = $(this).text();
                            $(this).value =document.getElementById(beer_abv).value;
                      });   
                    });

                }
    });
    
});

//BEER EBC
$(document).ready(function(){
    $.ajax( {
        type: "GET",
        url: "param.xml",
        dataType: "xml",
        success: function(xml) 
                 {
                   $(xml).find("param").each(
                     function()
                     {
                        $(this).find("beer_ebc").each(
                        function()
                        {    document.getElementById("beer_ebc").value = $(this).text();
                            $(this).value =document.getElementById(beer_ebc).value;
                      });   
                    });

                }
    });
    
});


$(document).ready(function(){
    $("#save_settings").click(function(){
        var valeur = $("#beer_name").val();
        $.post("beer_name",{
            beer_name: valeur
        });
    });
    $("#save_settings").click(function(){
        var valeur = $("#beer_ibu").val();
        $.post("beer_ibu",{
            beer_ibu: valeur
        });                
    });
    $("#save_settings").click(function(){
        var valeur = $("#beer_ebc").val();
        $.post("beer_ebc",{
            beer_ebc: valeur
        });
    });
    $("#save_settings").click(function(){
        var valeur = $("#beer_abv").val();
        $.post("beer_abv",{
            beer_abv: valeur
        });
    });
    $("#save_settings").click(function(){
        var valeur = $("#beer_sg").val();
        $.post("beer_sg",{
           beer_sg: valeur
        });
    });
});

// Solution a optimsier (lire le XML et changer la valeur de la balise pour le renvoyer dans le fichier...)
function XMLString ()
{
var file ="<?xml version='1.0' encoding='UTF-8' standalone='yes'?>";
file += "<param>";
file += "<beer_name id='1'>"+document.getElementById("beer_name").value+"</beer_name>";
file += "<beer_ibu id='2'>"+document.getElementById("beer_ibu").value+"</beer_ibu>";
file += "<beer_ebc id='3'>"+document.getElementById("beer_ebc").value+"</beer_ebc>";
file += "<beer_abv id='4'>"+document.getElementById("beer_abv").value+"</beer_abv>";
file += "<beer_sg id='5'>"+document.getElementById("beer_sg").value+"</beer_sg>";
file += "</param>";
return file;
}

// A trvailler pour détecter qui répond 1 et 2 
setInterval(function getData() {
    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("weight").innerHTML = this.responseText;
        }
        
    };

    xhttp.open("GET","weight", true);
    xhttp.send();
}, 2000);


function onButton() {
    var xhttp = new XMLHttpRequest();
    document.getElementById("scale_init").innerText = "Start..";
    //document.getElementById("OFF").innerText = "OFF";

    xhttp.open("GET", "scale_init", true);
    xhttp.send();
}

function offButton() {
    var xhttp = new XMLHttpRequest();
    document.getElementById("ON").innerText = "ON";
    document.getElementById("OFF").innerText = "OFF..";
    xhttp.open("GET", "off", true);
    xhttp.send();
}
