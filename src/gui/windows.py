import sys
import os
from PyQt5.QtCore import Qt, QItemSelection, QTimer
from PyQt5.QtWidgets import *
from PyQt5.QtGui import QIcon, QColor
from recept_pb2 import CocktailSetup, Recept

from gui.util import list_available_recipes, load_recipe, make_recipe, load_current_cocktail_setup, save_setup, get_progress, clean, adjust

import cobs

class RecipesListWidget(QListWidget):

    def __init__(self, display, parent= None):
        super().__init__(parent)

        self.selected_recipe = None
        self.display = display

    def selectionChanged(self, selected, deselected):
        self.selected_recipe = self.item(selected.first().indexes()[0].row()).data(Qt.UserRole)

        self.display.set_recipe(self.selected_recipe)

class MainWindow(QWidget):

     def __init__(self, parent = None):

         super().__init__(parent)

         self.defineGUI()

     def defineGUI(self):

         layout = QGridLayout(self)
         self.setGeometry(50, 50, 800, 500)
         self.setWindowTitle("Cocktail Maker")


         # Add a simple menu bar
         #menu_bar = QMenuBar(self)
         #menu_bar.setNativeMenuBar(False)
         #file_menu = menu_bar.addMenu("File")
         #file_menu.addAction("Exit")

         recipe_view = RecipeView(parent=self)
         recipe_view.setGeometry(150, 50, 150, 250)

         # List view
         recept_list = RecipesListWidget(recipe_view, self)

         for i in list_available_recipes("../../recipes"):

             # Load the recipe from file
             _, recipe = load_recipe(i)

             # List the recipe
             item = QListWidgetItem()
             item.setText(recipe['name'])

             if 'icon' in recipe:
                cocktail_icon = QIcon(os.path.join("../../img", recipe['icon']))
             else:
                cocktail_icon = QIcon("../../img/cocktail4.png")
             item.setIcon(cocktail_icon)
             item.setData(Qt.UserRole, recipe)
             recept_list.addItem(item)


         settings_button = QPushButton(icon= QIcon('../../img/settings.png'), text= "Cocktail Setup")
         settings_button.clicked.connect(lambda: CocktailSetupDialog(self).exec_())

         clean_button = QPushButton(icon= QIcon("../../img/Clean.jpg"), text= "Clean Machine")
         clean_button.clicked.connect(clean)

         adjust_button = QPushButton(icon= QIcon("../../img/filter.png"), text= "Adjust")
         adjust_button.clicked.connect(adjust)

         #layout.addWidget(QLabel("List of Cocktails:"), 0,0)
         layout.addWidget(recept_list, 0,0)
         layout.addWidget(recipe_view,0,1)
         layout.addWidget(settings_button,1,0)
         layout.addWidget(clean_button,1,1)
         layout.addWidget(adjust_button,2,1)

         #self.setLayout(layout)

class CocktailSetupDialog(QDialog):

    def __init__(self, parent):
        super().__init__(parent)
        self.defineGUI()

    def defineGUI(self):

        self.setWindowTitle("Cocktail Machine Setup")
        self.setWindowIcon(QIcon("../../img/settings.png"))
        self.setMinimumSize(300,300)

        layout = QGridLayout()
        self.setLayout(layout)

        # Read the cocktail setup from the machine
        setup = load_current_cocktail_setup()

        self.setup_table = QTableWidget(8,1,self)
        layout.addWidget(self.setup_table,0,0)

        for i in range(8):
            #label = QTableWidgetItem()
            #label.setText(str(i))
            #label.setFlags(Qt.ItemIsSelectable | Qt.ItemIsEnabled)
            liq = QTableWidgetItem()

            if i < len(setup.liquids):
                liq.setText(setup.liquids[i].name)
            self.setup_table.setItem(i, 0, liq)

        store_button = QPushButton("Save Setup", self)
        store_button.clicked.connect(self.store_setup)

        layout.addWidget(store_button,1,0)

    def store_setup(self):

        setup = CocktailSetup()

        for i in range(8):
            l = setup.liquids.add()
            l.name = self.setup_table.item(i,0).text()
            l.pump_id = i

        save_setup(setup)
        self.close()

class CocktailProgressWindow(QDialog):

    def __init__(self,parent, recipe):
        super().__init__(parent)

        # send a post to make
        answer = make_recipe(recipe)

        layout = QGridLayout(self)
        self.label = QLabel("Hallo")
        layout.addWidget(self.label, 0, 0)

        if(answer.status_code == 200):

            timer = QTimer(self)
            timer.setInterval(500)
            timer.timeout.connect(self.onTimer)
            timer.start(500)

            self.timer = timer
        else:
            self.label.setText(answer.text)


    def onTimer(self):
        progress, code = get_progress()
        self.label.setText(progress.decode("utf-8"))
        self.label.update()

        if code != 200:
            self.timer.stop()



class RecipeView(QWidget):

    def __init__(self, parent = None, recipe= None):
        super().__init__(parent)
        self.__recipe = recipe

        self.defineGUI()

    def defineGUI(self):

        layout = QGridLayout(self)
        self.__label = QLabel("Recipe Name")
        self.__button = QPushButton(icon= QIcon("../../img/shaker.png"), text= "Make")

        self.__ingredients_table = QTableWidget(self)

        self.__multiplier = QSlider(Qt.Horizontal,self)
        self.__multiplier.setMinimum(0)
        self.__multiplier.setMaximum(7)
        self.__multiplier.setSliderPosition(3)
        self.__multiplier.valueChanged.connect(self.multiplier_change)

        m = (self.__multiplier.value() + 1) * 0.25
        self.__amount_label = QLabel("Total amount:")

        layout.addWidget(self.__label, 0,0)
        layout.addWidget(QLabel("Ingredients:"),1,0)
        layout.addWidget(self.__ingredients_table, 2,0)
        layout.addWidget(self.__amount_label, 3,0)
        layout.addWidget(self.__multiplier, 4,0)
        layout.addWidget(self.__button, 5,0)

        self.__button.clicked.connect(self.make_requested)
        self.show()
        self.setVisible(False)

    def make_requested(self, arg):

        if self.__recipe is None:
            print("Nothing to make")
        else:
            m = (self.__multiplier.value() + 1) * 0.25
            # create a recipe from ingredients
            r = Recept()
            r.name = self.__recipe['name']
            for ing in self.__recipe['ingredients']:
                i = r.ingedients.add()
                i.name = ing['name']
                i.amount = ing['amount'] * m

            dia = CocktailProgressWindow(self, r)
            dia.exec_()

    def multiplier_change(self):
        m = (self.__multiplier.value() + 1) * 0.25
        self.__amount_label.setText(
            "Total Amount: %d ml (%.2fx)" % (sum(map(lambda d: d['amount'] * m, self.__recipe['ingredients'])), m))

        for idx, ing in enumerate(self.__recipe['ingredients']):
            self.__ingredients_table.item(idx,2).setText("%.2f ml" % (m * ing['amount']))

            if m != 1.0 :
                self.__ingredients_table.item(idx,2).setBackground(QColor("light yellow"))
            else:
                self.__ingredients_table.item(idx, 2).setBackground(QColor("white"))

    def set_recipe(self, recipe):

        self.setVisible(True)
        img_base_path = "../../img"

        # Find a good icon
        if 'icon' in recipe and recipe['icon']:
            icon_path = os.path.join(img_base_path, recipe['icon'])
        else:
            icon_path = os.path.join(img_base_path, 'cocktail4.png')

        self.__label.setText("""<html>
                                <div margin=auto>
                                <img src='%s' width=64 height=64>   
                                <font size=32>\t%s</font>
                                </div>
                                </html>""" % (icon_path, recipe["name"]))

        self.__ingredients_table.setRowCount(len(recipe['ingredients']))
        self.__ingredients_table.setColumnCount(3)
        for idx, ing in enumerate(recipe['ingredients']):

            label = QTableWidgetItem(ing['name'])
            label.setFlags(Qt.ItemIsSelectable | Qt.ItemIsEnabled)
            item = QTableWidgetItem("%.2f ml" % ing['amount'])
            item.setFlags(Qt.ItemIsSelectable | Qt.ItemIsEnabled)

            self.__ingredients_table.setItem(idx, 0, label)
            self.__ingredients_table.setItem(idx, 1, item)
            self.__ingredients_table.setItem(idx, 2, QTableWidgetItem(""))

        self.__ingredients_table.setHorizontalHeaderLabels(['Ingredient', 'Amount', "Adjusted"])
        self.__ingredients_table.setColumnWidth(0, (self.__ingredients_table.geometry().width() - 20) / 2)
        self.__ingredients_table.setColumnWidth(1, (self.__ingredients_table.geometry().width() - 20) / 4)
        self.__ingredients_table.setColumnWidth(2, (self.__ingredients_table.geometry().width() - 20) / 4)

        self.__recipe = recipe
        self.multiplier_change()




if __name__ == "__main__":
    app = QApplication(sys.argv)
    ex = MainWindow()
    ex.show()
    sys.exit(app.exec_())